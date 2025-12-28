//
// Created by benji on 17.12.25.
//
#include <pipewire/pipewire.h>
#include "Registry.h"

#include <chrono>
#include <thread>
#include "Utils.h"
struct PipeWire::Registry::PImpl {
    pw_registry *registry = nullptr;
    spa_hook registryListener{};
    pw_registry_events registryEvents{};
    std::chrono::steady_clock::time_point lastRegistryEvent;
    bool initialized = false;
    bool firstItemDetected = false;
};

namespace PipeWire {
    Registry::Registry(void *internalCore) {
        pImpl = std::make_unique<PImpl>();
        pImpl->registryEvents = {
            .version = PW_VERSION_REGISTRY_EVENTS,
            .global = [](void *data,
                         const uint32_t id,
                         uint32_t permissions,
                         const char *type,
                         uint32_t version,
                         const spa_dict *props) {
                const auto *self = static_cast<Registry *>(data);
                self->global(id, permissions, std::string(type), version, Utils::spaDictToMap(props));
            },
            .global_remove = [](void *data, uint32_t id) {
                const auto *self = static_cast<Registry *>(data);
                self->globalRemoved(id);
            }
        };
        pImpl->registry = pw_core_get_registry(static_cast<pw_core *>(internalCore), PW_VERSION_REGISTRY, 0);
        if (!pImpl->registry)
            throw RegistryConnectionError("Failed to get PipeWire registry");

        pw_registry_add_listener(
            pImpl->registry,
            &pImpl->registryListener,
            &pImpl->registryEvents,
            this
        );
        pImpl->lastRegistryEvent = std::chrono::steady_clock::now();
        std::thread([this] {
            using namespace std::chrono_literals;

            while (!pImpl->initialized ) {
                if (pImpl->firstItemDetected) {
                    std::this_thread::sleep_for(100ms);

                    auto now = std::chrono::steady_clock::now();
                    auto diff = now - pImpl->lastRegistryEvent;

                    // Wenn 300ms lang keine neuen Nodes/Ports/Links registriert wurden → fertig
                    if (diff > 300ms) {
                        pImpl->initialized = true;
                        this->initialized();
                        break;
                    }
                }
            }
        }).detach();
        // Adding event to update clock, Otherwhise initialized event wont work.
        // If global events are cleared, initalize will not work anymore.
        this->global += [this](ID, Permission, EventType, Version, Props) {
            if (!pImpl->initialized) {
                 pImpl->firstItemDetected=true;
                pImpl->lastRegistryEvent = std::chrono::steady_clock::now();
            }
        };
    }


    Registry::~Registry() {
        if (pImpl->registry) pw_proxy_destroy(reinterpret_cast<pw_proxy *>(pImpl->registry));
    }

    pw_registry * Registry::get() {
        return pImpl->registry;
    }

    pw_registry *Registry::operator*() {
        return get();
    }
} // PipeWire
