// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_BATCH_REQUEST_QUEUE_H
#define ICE_BATCH_REQUEST_QUEUE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/UniquePtr.h>

#include <Ice/BatchRequestInterceptor.h>
#include <Ice/BatchRequestQueueF.h>
#include <Ice/InstanceF.h>
#include <Ice/BasicStream.h>

namespace IceInternal
{

class BatchRequestQueue : public IceUtil::Shared, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    BatchRequestQueue(const InstancePtr&, bool);

    void prepareBatchRequest(BasicStream*);
    void finishBatchRequest(BasicStream*, const Ice::ObjectPrx&, const std::string&);
    void abortBatchRequest(BasicStream*);

    int swap(BasicStream*);

    void destroy(const Ice::LocalException&);
    bool isEmpty();

    void enqueueBatchRequest();

private:

    void waitStreamInUse(bool);

    Ice::BatchRequestInterceptorPtr _interceptor;
    BasicStream _batchStream;
    bool _batchStreamInUse;
    bool _batchStreamCanFlush;
    int _batchRequestNum;
    size_t _batchMarker;
    IceUtil::UniquePtr<Ice::LocalException> _exception;
    size_t _maxSize;
};

};

#endif
