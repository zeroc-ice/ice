//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPY_BATCH_REQUEST_INTERCEPTOR_H
#define ICEPY_BATCH_REQUEST_INTERCEPTOR_H

#include <Config.h>
#include <Util.h>
#include <Ice/BatchRequestInterceptor.h>

#include <memory>

namespace IcePy
{

extern PyTypeObject BatchRequestType;

bool initBatchRequest(PyObject*);

class BatchRequestInterceptor final
{
public:

    BatchRequestInterceptor(PyObject*);

    void enqueue(const Ice::BatchRequest&, int, int);

private:

    PyObjectHandle _interceptor;
};
using BatchRequestInterceptorPtr = std::shared_ptr<BatchRequestInterceptor>;

}

#endif
