//
// Created by benji on 17.12.25.
//

#ifndef RASPIHOST_CONTEXT_H
#define RASPIHOST_CONTEXT_H
#include <memory>
#include "Core.h"
namespace PipeWire {
    class Context {
    public:

        ~Context();
        Context(void *internalLoop);
        Context(const Context &Context) = delete;

        Context &operator=(const Context &Context) = delete;
        Core* getCore();
    private:


        struct PImpl;
        std::unique_ptr<PImpl> pImpl;
    };
} // PipeWire

#endif //RASPIHOST_CONTEXT_H
