// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/BatchRequestQueue.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>

using namespace std;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(BatchRequestQueue* p) { return p; }

namespace
{

const int udpOverhead = 20 + 8;

class BatchRequestI : public Ice::BatchRequest
{
public:

    BatchRequestI(BatchRequestQueue& queue, const Ice::ObjectPrx& proxy, const string& operation, int size) :
        _queue(queue), _proxy(proxy), _operation(operation), _size(size)
    {
    }

    virtual void
    enqueue() const
    {
        _queue.enqueueBatchRequest();
    }

    virtual int
    getSize() const
    {
        return _size;
    }

    virtual const std::string&
    getOperation() const
    {
        return _operation;
    }

    virtual const Ice::ObjectPrx&
    getProxy() const
    {
        return _proxy;
    }

private:

    BatchRequestQueue& _queue;
    const Ice::ObjectPrx& _proxy;
    const std::string& _operation;
    const int _size;
};

}

BatchRequestQueue::BatchRequestQueue(const InstancePtr& instance, bool datagram) :
    _interceptor(instance->initializationData().batchRequestInterceptor),
    _batchStream(instance.get(), Ice::currentProtocolEncoding),
    _batchStreamInUse(false),
    _batchStreamCanFlush(false),
    _batchRequestNum(0)
{
    _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
    _batchMarker = _batchStream.b.size();

    _maxSize = instance->batchAutoFlushSize();
    if(_maxSize > 0 && datagram)
    {
        const Ice::InitializationData& initData = instance->initializationData();
        size_t udpSndSize = initData.properties->getPropertyAsIntWithDefault("Ice.UDP.SndSize", 65535 - udpOverhead);
        if(udpSndSize < _maxSize)
        {
            _maxSize = udpSndSize;
        }
    }
}

void
BatchRequestQueue::prepareBatchRequest(BasicStream* os)
{
    Lock sync(*this);
    if(_exception.get())
    {
        _exception->ice_throw();
    }

    waitStreamInUse(false);
    _batchStreamInUse = true;
    _batchStream.swap(*os);
}

void
BatchRequestQueue::finishBatchRequest(BasicStream* os, const Ice::ObjectPrx& proxy, const std::string& operation)
{
    //
    // No need for synchronization, no other threads are supposed
    // to modify the queue since we set _batchStreamInUse to true.
    //
    assert(_batchStreamInUse);
    _batchStream.swap(*os);

    try
    {
        _batchStreamCanFlush = true; // Allow flush to proceed even if the stream is marked in use.

        if(_maxSize > 0 && _batchStream.b.size() >= _maxSize)
        {
            proxy->begin_ice_flushBatchRequests();
        }

        assert(_batchMarker < _batchStream.b.size());
        if(_interceptor)
        {
            BatchRequestI request(*this, proxy, operation, static_cast<int>(_batchStream.b.size() - _batchMarker));
            _interceptor->enqueue(request, _batchRequestNum, static_cast<int>(_batchMarker));
        }
        else
        {
            _batchMarker = _batchStream.b.size();
            ++_batchRequestNum;
        }

        Lock sync(*this);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _batchStreamCanFlush = false;
        notifyAll();
    }
    catch(const std::exception&)
    {
        Lock sync(*this);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _batchStreamCanFlush = false;
        notifyAll();
        throw;
    }
}

void
BatchRequestQueue::abortBatchRequest(BasicStream* os)
{
    Lock sync(*this);
    if(_batchStreamInUse)
    {
        _batchStream.swap(*os);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        notifyAll();
    }
}

int
BatchRequestQueue::swap(BasicStream* os)
{
    Lock sync(*this);
    if(_batchRequestNum == 0)
    {
        return 0;
    }

    waitStreamInUse(true);

    vector<Ice::Byte> lastRequest;
    if(_batchMarker < _batchStream.b.size())
    {
        vector<Ice::Byte>(_batchStream.b.begin() + _batchMarker, _batchStream.b.end()).swap(lastRequest);
        _batchStream.b.resize(_batchMarker);
    }

    int requestNum = _batchRequestNum;
    _batchStream.swap(*os);

    //
    // Reset the batch.
    //
    _batchRequestNum = 0;
    _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
    _batchMarker = _batchStream.b.size();
    if(!lastRequest.empty())
    {
        _batchStream.writeBlob(&lastRequest[0], lastRequest.size());
    }
    return requestNum;
}

void
BatchRequestQueue::destroy(const Ice::LocalException& ex)
{
    Lock sync(*this);
    _exception.reset(ex.ice_clone());
}

bool
BatchRequestQueue::isEmpty()
{
    Lock sync(*this);
    return _batchStream.b.size() == sizeof(requestBatchHdr);
}

void
BatchRequestQueue::waitStreamInUse(bool flush)
{
    while(_batchStreamInUse && !(flush && _batchStreamCanFlush))
    {
        wait();
    }
}

void
BatchRequestQueue::enqueueBatchRequest()
{
    assert(_batchMarker < _batchStream.b.size());
    _batchMarker = _batchStream.b.size();
    ++_batchRequestNum;
}
