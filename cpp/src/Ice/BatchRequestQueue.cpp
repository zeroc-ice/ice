// Copyright (c) ZeroC, Inc.

#include "BatchRequestQueue.h"
#include "Ice/Properties.h"
#include "Ice/Proxy.h"
#include "Instance.h"
#include "Reference.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    const int udpOverhead = 20 + 8;

    class BatchRequestI final : public Ice::BatchRequest
    {
    public:
        BatchRequestI(BatchRequestQueue& queue, const Ice::ObjectPrx& proxy, string_view operation, int size)
            : _queue(queue),
              _proxy(proxy),
              _operation(operation),
              _size(size)
        {
        }

        void enqueue() const final { _queue.enqueueBatchRequest(_proxy); }

        [[nodiscard]] int getSize() const final { return _size; }

        [[nodiscard]] string_view getOperation() const override { return _operation; }

        [[nodiscard]] const Ice::ObjectPrx& getProxy() const final { return _proxy; }

    private:
        BatchRequestQueue& _queue;
        const Ice::ObjectPrx& _proxy;
        const string_view _operation;
        const int _size;
    };
}

BatchRequest::~BatchRequest() = default; // avoid weak vtable

BatchRequestQueue::BatchRequestQueue(const InstancePtr& instance, bool datagram)
    : _interceptor(instance->initializationData().batchRequestInterceptor),
      _batchStream(instance.get(), currentProtocolEncoding)
{
    _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
    _batchMarker = _batchStream.b.size();

    _maxSize = instance->batchAutoFlushSize();
    if (_maxSize > 0 && datagram)
    {
        const Ice::InitializationData& initData = instance->initializationData();
        int32_t udpSndSize = initData.properties->getPropertyAsIntWithDefault("Ice.UDP.SndSize", 65535 - udpOverhead);
        if (udpSndSize < _maxSize)
        {
            _maxSize = udpSndSize;
        }
    }
}

void
BatchRequestQueue::prepareBatchRequest(OutputStream* os)
{
    unique_lock lock(_mutex);
    if (_exception)
    {
        rethrow_exception(_exception);
    }
    _conditionVariable.wait(lock, [this] { return !_batchStreamInUse; });

    _batchStreamInUse = true;
    _batchStream.swap(*os);
}

void
BatchRequestQueue::finishBatchRequest(OutputStream* os, const Ice::ObjectPrx& proxy, string_view operation)
{
    {
        // Bring the request stream back and become the owner so this thread's own auto-flush below can
        // re-enter swap(). swap() lets only _batchStreamOwnerId through while the stream is in use, and only
        // after passing that gate does it read the queue's bookkeeping; no other thread can touch the queue,
        // so the mutations below need no further synchronization.
        lock_guard lock(_mutex);
        assert(_batchStreamInUse);
        _batchStream.swap(*os);
        _batchStreamOwnerId = std::this_thread::get_id();
    }

    try
    {
        if (_maxSize > 0 && _batchStream.b.size() >= static_cast<size_t>(_maxSize))
        {
            proxy->ice_flushBatchRequestsAsync(nullptr); // auto-flush, don't wait for response
        }

        assert(_batchMarker < _batchStream.b.size());
        if (_interceptor)
        {
            BatchRequestI request(*this, proxy, operation, static_cast<int>(_batchStream.b.size() - _batchMarker));
            _interceptor(request, _batchRequestNum, static_cast<int>(_batchMarker));
        }
        else
        {
            optional<bool> compress = proxy->_getReference()->getCompressOverride();
            if (compress.has_value())
            {
                _batchCompress |= *compress;
            }
            _batchMarker = _batchStream.b.size();
            ++_batchRequestNum;
        }

        lock_guard lock(_mutex);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _batchStreamOwnerId = std::thread::id{};
        _conditionVariable.notify_all();
    }
    catch (...)
    {
        // Restore the queue to a usable state on any exception, including a non-std::exception escaping the
        // interceptor. Otherwise _batchStreamInUse would stay set and wedge the queue (all future
        // prepareBatchRequest/swap calls would block forever).
        lock_guard lock(_mutex);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _batchStreamOwnerId = std::thread::id{};
        _conditionVariable.notify_all();
        throw;
    }
}

void
BatchRequestQueue::abortBatchRequest(OutputStream* os)
{
    lock_guard lock(_mutex);
    if (_batchStreamInUse)
    {
        _batchStream.swap(*os);
        _batchStream.resize(_batchMarker);
        _batchStreamInUse = false;
        _conditionVariable.notify_all();
    }
}

int
BatchRequestQueue::swap(OutputStream* os, bool& compress) noexcept
{
    unique_lock lock(_mutex);

    // Only the thread that currently owns the in-use stream may bypass the wait, to run its own auto-flush;
    // every other caller waits for the in-progress batch request to finish. (The owner can't change while we
    // block here: a thread becomes the owner only inside finishBatchRequest, never while parked in this wait.)
    if (_batchStreamOwnerId != std::this_thread::get_id())
    {
        _conditionVariable.wait(lock, [this] { return !_batchStreamInUse; });
    }

    // Read the bookkeeping only after passing the gate above: finishBatchRequest mutates these scalars
    // without the lock while it owns the in-use stream, so reading _batchRequestNum before the wait would
    // race those writes.
    if (_batchRequestNum == 0)
    {
        return 0;
    }

    vector<byte> lastRequest;
    if (_batchMarker < _batchStream.b.size())
    {
        vector<byte>(_batchStream.b.begin() + _batchMarker, _batchStream.b.end()).swap(lastRequest);
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
    if (!lastRequest.empty())
    {
        _batchStream.writeBlob(lastRequest.data(), lastRequest.size());
    }
    return requestNum;
}

void
BatchRequestQueue::destroy(std::exception_ptr ex)
{
    lock_guard lock(_mutex);
    _exception = ex;
}

void
BatchRequestQueue::enqueueBatchRequest(const Ice::ObjectPrx& proxy)
{
    assert(_batchMarker < _batchStream.b.size());
    optional<bool> compress = proxy->_getReference()->getCompressOverride();
    if (compress.has_value())
    {
        _batchCompress |= *compress;
    }
    _batchMarker = _batchStream.b.size();
    ++_batchRequestNum;
}
