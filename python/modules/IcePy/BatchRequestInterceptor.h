// Copyright (c) ZeroC, Inc.

#ifndef ICEPY_BATCH_REQUEST_INTERCEPTOR_H
#define ICEPY_BATCH_REQUEST_INTERCEPTOR_H

#include "Config.h"
#include "Ice/Ice.h"
#include "Util.h"

#include <memory>

namespace IcePy
{
    extern PyTypeObject BatchRequestType;

    bool initBatchRequest(PyObject*);

    class BatchRequestInterceptorWrapper final
    {
    public:
        BatchRequestInterceptorWrapper(PyObject*);

        void enqueue(const Ice::BatchRequest&, int, int);

    private:
        PyObjectHandle _interceptor;
    };
}

#endif
