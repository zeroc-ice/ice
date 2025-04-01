// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_EXECUTOR_H
#define ICEPY_EXECUTOR_H

#include "Config.h"
#include "Ice/CommunicatorF.h"
#include "Util.h"

namespace IcePy
{
    bool initExecutor(PyObject*);

    class Executor final
    {
    public:
        Executor(PyObject*);

        void setCommunicator(const Ice::CommunicatorPtr&);

        void execute(std::function<void()> call, const Ice::ConnectionPtr&);

    private:
        PyObjectHandle _executor;
        Ice::CommunicatorPtr _communicator;
    };
    using ExecutorPtr = std::shared_ptr<Executor>;
}

#endif
