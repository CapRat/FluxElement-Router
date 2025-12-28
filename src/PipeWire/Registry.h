//
// Created by benji on 17.12.25.
//

#ifndef RASPIHOST_REGISTRY_H
#define RASPIHOST_REGISTRY_H
#include <map>
#include <memory>
#include <utility>

#include "../Utils/Event.h"
struct pw_registry;
struct spa_dict;

namespace PipeWire {
    using ID = const uint32_t;
    using Permission = uint32_t;
    using EventType = std::string;
    using Version = uint32_t;
    using Props = std::map<std::string, std::string>;

    class RegistryConnectionError : public std::exception {
        std::string message;

    public:
        explicit RegistryConnectionError(std::string msg) : message(std::move(msg)) {
        }

        [[nodiscard]] const char *what() const noexcept override {
            return message.c_str();
        }
    };

    class Registry {
    public:
        explicit Registry(void *internalCore);

        ~Registry();

        Registry(const Registry &other) = delete;

        Registry &operator=(const Registry &other) = delete;

        Event<void(ID, Permission, EventType, Version, Props)> global;
        Event<void(ID)> globalRemoved;
        Event<void()> initialized;
        pw_registry * get();
        pw_registry *operator*();

    private:
        struct PImpl;
        std::unique_ptr<PImpl> pImpl;
    };
} // PipeWire

#endif //RASPIHOST_REGISTRY_H
