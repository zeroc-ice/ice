// Copyright (c) ZeroC, Inc.

#include "CollocatedRequestHandler.h"
#include "Endian.h"
#include "Ice/OutgoingAsync.h"
#include "Instance.h"
#include "ObjectAdapterI.h"
#include "Reference.h"
#include "ThreadPool.h"
#include "TraceLevels.h"
#include "TraceUtil.h"

#include <algorithm>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    void fillInValue(OutputStream* os, int pos, int32_t value)
    {
        const byte* p = reinterpret_cast<const byte*>(&value);
        if constexpr (endian::native == endian::big)
        {
            reverse_copy(p, p + sizeof(std::int32_t), os->b.begin() + pos);
        }
        else
        {
            copy(p, p + sizeof(std::int32_t), os->b.begin() + pos);
        }
    }
}

CollocatedRequestHandler::CollocatedRequestHandler(const ReferencePtr& ref, const ObjectAdapterPtr& adapter)
    : RequestHandler(ref),
      _adapter(dynamic_pointer_cast<ObjectAdapterI>(adapter)),
      _hasExecutor(
          _reference->getInstance()->initializationData().executor != nullptr), // Cached for better performance.
      _logger(_reference->getInstance()->initializationData().logger),          // Cached for better performance.
      _traceLevels(_reference->getInstance()->traceLevels())
{
}

CollocatedRequestHandler::~CollocatedRequestHandler() = default;

AsyncStatus
CollocatedRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& outAsync)
{
    return outAsync->invokeCollocated(this);
}

void
CollocatedRequestHandler::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, exception_ptr ex)
{
    lock_guard<mutex> lock(_mutex);

    auto p = _sendAsyncRequests.find(outAsync);
    if (p != _sendAsyncRequests.end())
    {
        if (p->second > 0)
        {
            _asyncRequests.erase(p->second);
        }
        _sendAsyncRequests.erase(p);
        if (outAsync->exception(ex))
        {
            outAsync->invokeExceptionAsync();
        }
        _adapter->decDirectCount(); // dispatchAll won't be called, decrease the direct count.
        return;
    }

    OutgoingAsyncPtr o = dynamic_pointer_cast<OutgoingAsync>(outAsync);
    if (o)
    {
        for (auto q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
        {
            if (q->second.get() == o.get())
            {
                _asyncRequests.erase(q);
                if (outAsync->exception(ex))
                {
                    outAsync->invokeExceptionAsync();
                }
                return;
            }
        }
    }
}

AsyncStatus
CollocatedRequestHandler::invokeAsyncRequest(OutgoingAsyncBase* outAsync, int batchRequestCount, bool synchronous)
{
    // Increase the direct count to prevent the thread pool from being destroyed before dispatchAll is called. This will
    // also throw if the object adapter has been deactivated.
    _adapter->incDirectCount();

    int requestId = 0;
    try
    {
        {
            lock_guard<mutex> lock(_mutex);

            //
            // This will throw if the request is canceled
            //
            outAsync->cancelable(shared_from_this());

            if (_response)
            {
                requestId = ++_requestId;
                _asyncRequests.insert(make_pair(requestId, outAsync->shared_from_this()));
            }

            _sendAsyncRequests.insert(make_pair(outAsync->shared_from_this(), requestId));
        }

        OutputStream* os = outAsync->getOs();
        if (_traceLevels->protocol >= 1)
        {
            fillInValue(os, 10, static_cast<int32_t>(os->b.size()));
            if (requestId > 0)
            {
                fillInValue(os, headerSize, requestId);
            }
            else if (batchRequestCount > 0)
            {
                fillInValue(os, headerSize, batchRequestCount);
            }
            traceSend(*os, _reference->getInstance(), nullptr, _logger, _traceLevels);
        }

        outAsync->attachCollocatedObserver(_adapter, requestId);

        InputStream is{_reference->getInstance().get(), os->getEncoding(), *os, false};

        if (batchRequestCount > 0)
        {
            is.pos(sizeof(requestBatchHdr));
        }
        else
        {
            is.pos(sizeof(requestHdr));
        }

        int dispatchCount = batchRequestCount == 0 ? 1 : batchRequestCount;

        //
        // Make sure to hold a reference on this handler while the call is being
        // dispatched. Otherwise, the handler could be deleted during the dispatch
        // if a retry occurs.
        //
        auto self = shared_from_this();

        if (!synchronous || !_response || _reference->getInvocationTimeout() > 0ms)
        {
            auto stream = make_shared<InputStream>(_reference->getInstance().get(), currentProtocolEncoding);
            is.swap(*stream);

            // Don't invoke from the user thread if async or invocation timeout is set
            _adapter->getThreadPool()->execute(
                [self, outAsync = outAsync->shared_from_this(), stream, requestId, dispatchCount]()
                {
                    if (self->sentAsync(outAsync.get()))
                    {
                        self->dispatchAll(*stream, requestId, dispatchCount);
                    }
                },
                nullptr);
        }
        else if (_hasExecutor)
        {
            auto stream = make_shared<InputStream>(_reference->getInstance().get(), currentProtocolEncoding);
            is.swap(*stream);

            _adapter->getThreadPool()->executeFromThisThread(
                [self, outAsync = outAsync->shared_from_this(), stream, requestId, dispatchCount]()
                {
                    if (self->sentAsync(outAsync.get()))
                    {
                        self->dispatchAll(*stream, requestId, dispatchCount);
                    }
                },
                nullptr);
        }
        else // Optimization: directly call dispatchAll if there's no custom executor.
        {
            if (sentAsync(outAsync))
            {
                dispatchAll(is, requestId, dispatchCount);
            }
        }
    }
    catch (...)
    {
        // Decrement the direct count if any exception is thrown synchronously.
        _adapter->decDirectCount();
        throw;
    }
    return AsyncStatusQueued;
}

void
CollocatedRequestHandler::dispatchException(int32_t requestId, exception_ptr ex)
{
    handleException(requestId, ex);
    _adapter->decDirectCount();
}

ConnectionIPtr
CollocatedRequestHandler::getConnection()
{
    return nullptr;
}

bool
CollocatedRequestHandler::sentAsync(OutgoingAsyncBase* outAsync)
{
    {
        lock_guard<mutex> lock(_mutex);
        if (_sendAsyncRequests.erase(outAsync->shared_from_this()) == 0)
        {
            return false; // The request timed-out.
        }

        if (!outAsync->sent())
        {
            return true;
        }
    }
    outAsync->invokeSent();
    return true;
}

void
CollocatedRequestHandler::dispatchAll(InputStream& is, int32_t requestId, int32_t requestCount)
{
    try
    {
        while (requestCount > 0)
        {
            // Increase the direct count for the dispatch. We increase it again here for
            // each dispatch. It's important for the direct count to be > 0 until the last
            // collocated request response is sent to make sure the thread pool isn't
            // destroyed before. It's decremented when processing the response.
            try
            {
                _adapter->incDirectCount();
            }
            catch (const ObjectAdapterDestroyedException&)
            {
                handleException(requestId, current_exception());
                break;
            }

            IncomingRequest request{requestId, nullptr, _adapter, is, requestCount};

            try
            {
                _adapter->dispatchPipeline()->dispatch(
                    request,
                    [self = shared_from_this()](OutgoingResponse response)
                    { self->sendResponse(std::move(response)); });
            }
            catch (...)
            {
                sendResponse(makeOutgoingResponse(current_exception(), request.current()));
            }

            --requestCount;
        }
    }
    catch (...)
    {
        dispatchException(requestId, current_exception()); // Fatal invocation exception
    }

    _adapter->decDirectCount();
}

void
CollocatedRequestHandler::handleException(int32_t requestId, std::exception_ptr ex)
{
    if (requestId == 0)
    {
        return; // Ignore exception for oneway messages.
    }

    OutgoingAsyncBasePtr outAsync;
    {
        lock_guard<mutex> lock(_mutex);

        auto q = _asyncRequests.find(requestId);
        if (q != _asyncRequests.end())
        {
            if (q->second->exception(ex))
            {
                outAsync = q->second;
            }
            _asyncRequests.erase(q);
        }
    }

    if (outAsync)
    {
        // We invoke the exception using a thread-pool thread. If the invocation is a lambda async invocation, we want
        // the callbacks to execute in a thread-pool thread - never in the application thread that sent the exception
        // via AMD.
        outAsync->invokeExceptionAsync();
    }
}

void
CollocatedRequestHandler::sendResponse(OutgoingResponse response)
{
    try
    {
        if (_response)
        {
            OutgoingAsyncBasePtr outAsync;
            {
                lock_guard<mutex> lock(_mutex);
                assert(_response);

                auto os = &response.outputStream();

                if (_traceLevels->protocol >= 1)
                {
                    fillInValue(os, 10, static_cast<int32_t>(os->b.size()));
                }

                InputStream is(
                    _reference->getInstance().get(),
                    os->getEncoding(),
                    *os,
                    true); // Adopting the OutputStream's buffer.
                is.pos(sizeof(replyHdr) + 4);

                if (_traceLevels->protocol >= 1)
                {
                    traceRecv(is, nullptr, _logger, _traceLevels);
                }

                auto q = _asyncRequests.find(response.current().requestId);
                if (q != _asyncRequests.end())
                {
                    is.swap(*q->second->getIs());
                    if (q->second->response())
                    {
                        outAsync = q->second;
                    }
                    _asyncRequests.erase(q);
                }
            }

            if (outAsync)
            {
                // We invoke the response using a thread-pool thread. If the invocation is a lambda async invocation, we
                // want the callbacks to execute in a thread-pool thread - never in the application thread that sent the
                // response via AMD.
                outAsync->invokeResponseAsync();
            }
        }

        _adapter->decDirectCount();
    }
    catch (...)
    {
        // Fatal invocation exception
        dispatchException(response.current().requestId, current_exception());
    }
}
