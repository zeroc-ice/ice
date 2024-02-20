//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/BatchRequestQueue.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/Reference.h>
#include "Ice/Proxy.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

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
    _batchCompress(false),
    _batchRequestNum(0)
{
    _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
    _batchMarker = _batchStream.b.size();

    _maxSize = instance->batchAutoFlushSize();
    if(_maxSize > 0 && datagram)
    {
        const Ice::InitializationData& initData = instance->initializationData();
        size_t udpSndSize = static_cast<size_t>(initData.properties->getPropertyAsIntWithDefault("Ice.UDP.SndSize", 65535 - udpOverhead));
        if(udpSndSize < _maxSize)
        {
            _maxSize = udpSndSize;
        }
    }
}

void
BatchRequestQueue::prepareBatchRequest(OutputStream* os)
{
    unique_lock lock(_mutex);
    if(_exception)
    {
        rethrow_exception(_exception);
    }
    _conditionVariable.wait(lock, [this] { return !_batchStreamInUse; });

    _batchStreamInUse = true;
    _batchStream.swap(*os);
}

void
BatchRequestQueue::finishBatchRequest(OutputStream* os,
                                      const Ice::ObjectPrx& proxy,
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
            proxy->ice_flushBatchRequestsAsync();
        }

        assert(_batchMarker < _batchStream.b.size());
        if(_interceptor)
        {
            BatchRequestI request(*this, proxy, operation, static_cast<int>(_batchStream.b.size() - _batchMarker));
            _interceptor(request, _batchRequestNum, static_cast<int>(_batchMarker));
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

        lock_guard lock(_mutex);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _batchStreamCanFlush = false;
        _conditionVariable.notify_all();
    }
    catch(const std::exception&)
    {
        lock_guard lock(_mutex);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _batchStreamCanFlush = false;
        _conditionVariable.notify_all();
        throw;
    }
}

void
BatchRequestQueue::abortBatchRequest(OutputStream* os)
{
    lock_guard lock(_mutex);
    if(_batchStreamInUse)
    {
        _batchStream.swap(*os);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _conditionVariable.notify_all();
    }
}

int
BatchRequestQueue::swap(OutputStream* os, bool& compress)
{
    unique_lock lock(_mutex);
    if(_batchRequestNum == 0)
    {
        return 0;
    }

    _conditionVariable.wait(lock, [this] { return !_batchStreamInUse || _batchStreamCanFlush; });

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
BatchRequestQueue::destroy(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _exception = ex;
}

bool
BatchRequestQueue::isEmpty()
{
    lock_guard lock(_mutex);
    return _batchStream.b.size() == sizeof(requestBatchHdr);
}

void
BatchRequestQueue::enqueueBatchRequest(const Ice::ObjectPrx& proxy)
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
