// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPY_BATCH_REQUEST_INTERCEPTOR_H
#define ICEPY_BATCH_REQUEST_INTERCEPTOR_H

#include <Config.h>
#include <Util.h>
#include <Ice/BatchRequestInterceptor.h>

namespace IcePy
{

extern PyTypeObject BatchRequestType;

bool initBatchRequest(PyObject*);

class BatchRequestInterceptor : public Ice::BatchRequestInterceptor
{
public:

    BatchRequestInterceptor(PyObject*);

    virtual void enqueue(const Ice::BatchRequest&, int, int);

private:

    PyObjectHandle _interceptor;
};
typedef IceUtil::Handle<BatchRequestInterceptor> BatchRequestInterceptorPtr;

}

#endif
