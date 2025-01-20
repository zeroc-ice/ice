// Copyright (c) ZeroC, Inc.

#include "CommunicatorFlushBatchAsync.h"
#include "BatchRequestQueue.h"
#include "ConnectionFactory.h"
#include "ConnectionI.h"
#include "Instance.h"
#include "ObjectAdapterFactory.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

CommunicatorFlushBatchAsync::~CommunicatorFlushBatchAsync() = default; // Out of line to avoid weak vtable

CommunicatorFlushBatchAsync::CommunicatorFlushBatchAsync(const InstancePtr& instance) : OutgoingAsyncBase(instance)
{
    //
    // _useCount is initialized to 1 to prevent premature callbacks.
    // The caller must invoke ready() after all flush requests have
    // been initiated.
    //
    _useCount = 1;
}

void
CommunicatorFlushBatchAsync::flushConnection(const ConnectionIPtr& con, Ice::CompressBatch compressBatch)
{
    class FlushBatch final : public OutgoingAsyncBase
    {
    public:
        FlushBatch(CommunicatorFlushBatchAsyncPtr outAsync, const InstancePtr& instance, InvocationObserver& observer)
            : OutgoingAsyncBase(instance),
              _outAsync(std::move(outAsync)),
              _parentObserver(observer)
        {
        }

        bool sent() override
        {
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        bool exception(std::exception_ptr ex) override
        {
            _childObserver.failed(getExceptionId(ex));
            _childObserver.detach();
            _outAsync->check(false);
            return false;
        }

        InvocationObserver& getObserver() override { return _parentObserver; }

        bool handleSent(bool, bool) noexcept final { return false; }

        bool handleException(std::exception_ptr) noexcept final { return false; }

        bool handleResponse(bool) final { return false; }

        void handleInvokeSent(bool, OutgoingAsyncBase*) const final { assert(false); }

        void handleInvokeException(std::exception_ptr, OutgoingAsyncBase*) const final { assert(false); }

        void handleInvokeResponse(bool, OutgoingAsyncBase*) const final { assert(false); }

    private:
        const CommunicatorFlushBatchAsyncPtr _outAsync;
        InvocationObserver& _parentObserver;
    };

    {
        Lock sync(_m);
        ++_useCount;
    }

    try
    {
        OutgoingAsyncBasePtr flushBatch = make_shared<FlushBatch>(shared_from_this(), _instance, _observer);
        bool compress;
        int batchRequestNum = con->getBatchRequestQueue()->swap(flushBatch->getOs(), compress);
        if (batchRequestNum == 0)
        {
            flushBatch->sent();
        }
        else
        {
            if (compressBatch == CompressBatch::Yes)
            {
                compress = true;
            }
            else if (compressBatch == CompressBatch::No)
            {
                compress = false;
            }
            con->sendAsyncRequest(flushBatch, compress, false, batchRequestNum);
        }
    }
    catch (const LocalException&)
    {
        check(false);
        throw;
    }
}

void
CommunicatorFlushBatchAsync::invoke(string_view operation, CompressBatch compressBatch)
{
    _observer.attach(_instance.get(), operation);
    _instance->outgoingConnectionFactory()->flushAsyncBatchRequests(shared_from_this(), compressBatch);
    _instance->objectAdapterFactory()->flushAsyncBatchRequests(shared_from_this(), compressBatch);
    check(true);
}

void
CommunicatorFlushBatchAsync::check(bool userThread)
{
    {
        Lock sync(_m);
        assert(_useCount > 0);
        if (--_useCount > 0)
        {
            return;
        }
    }

    if (sentImpl(true))
    {
        if (userThread)
        {
            _sentSynchronously = true;
            invokeSent();
        }
        else
        {
            invokeSentAsync();
        }
    }
}
