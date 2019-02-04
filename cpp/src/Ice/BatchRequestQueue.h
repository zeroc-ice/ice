//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_BATCH_REQUEST_QUEUE_H
#define ICE_BATCH_REQUEST_QUEUE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/UniquePtr.h>

#include <Ice/BatchRequestInterceptor.h>
#include <Ice/BatchRequestQueueF.h>
#include <Ice/InstanceF.h>
#include <Ice/OutputStream.h>

namespace IceInternal
{

class BatchRequestQueue : public IceUtil::Shared, private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    BatchRequestQueue(const InstancePtr&, bool);

    void prepareBatchRequest(Ice::OutputStream*);
    void finishBatchRequest(Ice::OutputStream*, const Ice::ObjectPrxPtr&, const std::string&);
    void abortBatchRequest(Ice::OutputStream*);

    int swap(Ice::OutputStream*, bool&);

    void destroy(const Ice::LocalException&);
    bool isEmpty();

    void enqueueBatchRequest(const Ice::ObjectPrxPtr&);

private:

    void waitStreamInUse(bool);

#ifdef ICE_CPP11_MAPPING
    std::function<void(const Ice::BatchRequest&, int, int)> _interceptor;
#else
    Ice::BatchRequestInterceptorPtr _interceptor;
#endif
    Ice::OutputStream _batchStream;
    bool _batchStreamInUse;
    bool _batchStreamCanFlush;
    bool _batchCompress;
    int _batchRequestNum;
    size_t _batchMarker;
    IceInternal::UniquePtr<Ice::LocalException> _exception;
    size_t _maxSize;
};

};

#endif
