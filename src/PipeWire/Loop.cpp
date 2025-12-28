//
// Created by benji on 17.12.25.
//

#include "Loop.h"

#include <memory>
#include <thread>
#include <utility>
#include <oneapi/tbb/detail/_task.h>
#include <pipewire/pipewire.h>

struct StoreFunction {
    std::function<int()> fnc;
};

struct PipeWire::Loop::PImpl {
    pw_main_loop *loop = nullptr;
    spa_dict properties;
    std::thread loopThread;
    std::unique_ptr<Context> context;
};

namespace PipeWire {
    Loop::Loop() {
        pImpl = std::make_unique<PImpl>();
        pImpl->loop = pw_main_loop_new(&pImpl->properties);
        pImpl->context = std::make_unique<Context>(pImpl->loop);
    }

    Loop::~Loop() {
        this->stop();
        if (pImpl->loop) pw_main_loop_destroy(pImpl->loop);
    }

    void Loop::start() {
        pImpl->loopThread = std::thread([this]() {
            pw_main_loop_run(pImpl->loop);
        });
    }

    void Loop::stop() {
        pImpl->context.reset();
        if (pImpl->loop)
            pw_main_loop_quit(pImpl->loop);
        if (pImpl->loopThread.joinable())
            pImpl->loopThread.join();
    }

    Context *Loop::getContext() {
        return pImpl->context.get();
    }

    int Loop::invokeFunctionInLoop(std::function<int()> function) {
        // ReSharper disable once CppDFAMemoryLeak
        const auto newFnc = new StoreFunction{std::move(function)};
        const auto retCode = pw_loop_invoke(
            pw_main_loop_get_loop(get()),
            [](struct spa_loop *loop, bool async, uint32_t seq, const void *data, size_t size,
               void *user_data)-> int {
                const auto transferObj = static_cast<StoreFunction*>(user_data);
                const auto ret= transferObj->fnc();
                delete transferObj;
                return ret;
            },
            0,
            nullptr,
            0,
            false,
            newFnc
        );
        return retCode;
    }

    pw_main_loop *Loop::get() {
        return pImpl->loop;
    }
} // PipeWire
