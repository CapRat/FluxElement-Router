//
// Created by benji on 17.12.25.
//

#ifndef RASPIHOST_LOOP_H
#define RASPIHOST_LOOP_H
#include <memory>
struct pw_main_loop;

#include "Context.h"
namespace PipeWire {
    class Loop {
    public:
        Loop();

        ~Loop();

        Loop(const Loop &other) = delete;

        Loop &operator=(const Loop &other)=delete;

        void start();

        void stop();

        Context* getContext();


        int invokeFunctionInLoop(std::function<int()> function);

        pw_main_loop *get();
    private:
        struct PImpl;
        std::unique_ptr<PImpl> pImpl;
    };


} // PipeWire

#endif //RASPIHOST_LOOP_H
