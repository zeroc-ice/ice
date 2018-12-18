// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/BatchRequestQueue.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/Reference.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceUtil::Shared* IceInternal::upCast(BatchRequestQueue* p) { return p; }

namespace
{

const int udpOverhead = 20 + 8;

class BatchRequestI : public Ice::BatchRequest
{
public:

    BatchRequestI(BatchRequestQueue& queue, const Ice::ObjectPrxPtr& proxy, const string& operation, int size) :
        _queue(queue), _proxy(proxy), _operation(operation), _size(size)
    {
    }

    virtual void
    enqueue() const
    {
        _queue.enqueueBatchRequest(_proxy);
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

    virtual const Ice::ObjectPrxPtr&
    getProxy() const
    {
        return _proxy;
    }

private:

    BatchRequestQueue& _queue;
    const Ice::ObjectPrxPtr& _proxy;
    const std::string& _operation;
    const int _size;
};

}

BatchRequestQueue::BatchRequestQueue(const InstancePtr& instance, bool datagram) :
    _interceptor(instance->initializationData().batchRequestInterceptor),
    _batchStream(instance.get(), Ice::currentProtocolEncoding),
    _batchStreamInUse(false),
    _batchStreamCanFlush(false),
    _batchCompress(false),
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
BatchRequestQueue::prepareBatchRequest(OutputStream* os)
{
    Lock sync(*this);
    if(_exception)
    {
        _exception->ice_throw();
    }
    waitStreamInUse(false);
    _batchStreamInUse = true;
    _batchStream.swap(*os);
}

void
BatchRequestQueue::finishBatchRequest(OutputStream* os,
                                      const Ice::ObjectPrxPtr& proxy,
                                      const std::string& operation)
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
#ifdef ICE_CPP11_MAPPING
            proxy->ice_flushBatchRequestsAsync();
#else
            proxy->begin_ice_flushBatchRequests();
#endif
        }

        assert(_batchMarker < _batchStream.b.size());
        if(_interceptor)
        {
            BatchRequestI request(*this, proxy, operation, static_cast<int>(_batchStream.b.size() - _batchMarker));
#ifdef ICE_CPP11_MAPPING
            _interceptor(request, _batchRequestNum, static_cast<int>(_batchMarker));
#else
            _interceptor->enqueue(request, _batchRequestNum, static_cast<int>(_batchMarker));
#endif
        }
        else
        {
            bool compress;
            if(proxy->_getReference()->getCompressOverride(compress))
            {
                _batchCompress |= compress;
            }
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
BatchRequestQueue::abortBatchRequest(OutputStream* os)
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
BatchRequestQueue::swap(OutputStream* os, bool& compress)
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
    compress = _batchCompress;

    //
    // Reset the batch.
    //
    _batchRequestNum = 0;
    _batchCompress = false;
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
#ifdef ICE_CPP11_MAPPING
    _exception = ex.ice_clone();
#else
    _exception.reset(ex.ice_clone());
#endif
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
BatchRequestQueue::enqueueBatchRequest(const Ice::ObjectPrxPtr& proxy)
{
    assert(_batchMarker < _batchStream.b.size());
    bool compress;
    if(proxy->_getReference()->getCompressOverride(compress))
    {
        _batchCompress |= compress;
    }
    _batchMarker = _batchStream.b.size();
    ++_batchRequestNum;
}
