//
// Created by benji on 17.12.25.
//

#include "Core.h"
#include <pipewire/pipewire.h>

struct PipeWire::Core::PImpl {
    pw_core *core = nullptr;
    spa_hook coreListener{};
    pw_core_events core_events{};
    std::unique_ptr<Registry> registry;
};

namespace PipeWire {
    Core::Core(void *internalContext) {
        pImpl = std::make_unique<PImpl>();
        pImpl->core_events = {
            .version = PW_VERSION_CORE_EVENTS,
            .info = [](void *data, const pw_core_info *info) {
                // auto *self = static_cast<PipeWireManager *>(data);
            },
            .done = [](void *data, uint32_t id, int seq) {
                // auto *self = static_cast<PipeWireManager *>(data);
            },

            .ping = nullptr,
            .error = nullptr,
            .remove_id = nullptr,
            .bound_id = nullptr,
            .add_mem = nullptr,
            .remove_mem = nullptr,
            .bound_props = nullptr,
        };
        pImpl->core = pw_context_connect(static_cast<pw_context *>(internalContext), nullptr, 0);
        if (!pImpl->core)
            throw CoreConnectionError("Failed to connect to PipeWire core");
        pw_core_add_listener(
            pImpl->core,
            &pImpl->coreListener,
            &pImpl->core_events,
            this);
        pImpl->registry = std::make_unique<Registry>(pImpl->core);
    }

    Core::~Core() {
        pImpl->registry.reset();
        if (pImpl->core) pw_core_disconnect(pImpl->core);
    }

    pw_core * Core::get() {
        return pImpl->core;
    }

    Registry * Core::getRegistry() {
        return pImpl->registry.get();
    }
} // PipeWire
