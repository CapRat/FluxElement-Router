//
// Created by benji on 17.12.25.
//

#ifndef RASPIHOST_CORE_H
#define RASPIHOST_CORE_H
#include <memory>
#include <utility>
#include "Registry.h"
#include "src/PipeWireManager.h"

namespace PipeWire {
    class CoreConnectionError : public std::exception {
        std::string message;

    public:
        explicit CoreConnectionError(std::string msg) : message(std::move(msg)) {
        }

        [[nodiscard]] const char *what() const noexcept override {
            return message.c_str();
        }
    };

    class Core {
    public:
        explicit Core(void *internalContext);

        ~Core();


        Core(const Core &core) = delete;

        Core &operator=(const Core &core) = delete;

        pw_core *get();

        Registry *getRegistry();

    private:
        struct PImpl;
        std::unique_ptr<PImpl> pImpl;
    };
} // PipeWire

#endif //RASPIHOST_CORE_H
