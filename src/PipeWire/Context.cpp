//
// Created by benji on 17.12.25.
//
#include <pipewire/pipewire.h>
#include "Context.h"

#include <oneapi/tbb/detail/_task.h>

struct PipeWire::Context::PImpl {
    pw_context *context = nullptr;
    pw_context_events context_events{};
    spa_hook event_listener;
    std::unique_ptr<Core> core;
};

namespace PipeWire {
    Context::Context(void *internalLoop) {
        pImpl = std::make_unique<PImpl>();
        pImpl->context_events = {
            .version = PW_VERSION_CONTEXT_EVENTS,
            .destroy = [](void *data) {
            },
            .free = [](void *data) {
            },
            .check_access = [](void *data, pw_impl_client *client) {
            },
            .global_added = [](void *data, pw_global *global) {
            },
            .global_removed = [](void *data, pw_global *global) {
            },
            .driver_added = [](void *data, pw_impl_node *node) {
            },
            .driver_removed = [](void *data, pw_impl_node *node) {
            },
        };
        pw_main_loop *mainLoop = static_cast<pw_main_loop *>(internalLoop);
        pImpl->context = pw_context_new(pw_main_loop_get_loop(mainLoop), nullptr, 0);
        pw_context_add_listener(pImpl->context, &pImpl->event_listener, &pImpl->context_events, this);
        pImpl->core = std::make_unique<Core>(pImpl->context);
    }

    Core *Context::getCore() {
        return pImpl->core.get();
    }


    Context::~Context() {
        pImpl->core.reset();
        if (pImpl->context) pw_context_destroy(pImpl->context);
    }
} // PipeWire
