// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/ObjectAdapterI.h>
#include <Ice/ThreadPool.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/TraceLevels.h>
#include <Ice/OutgoingAsync.h>

#include <Ice/TraceUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class InvokeAllAsync : public DispatchWorkItem
{
public:

    InvokeAllAsync(const OutgoingAsyncBasePtr& outAsync,
                   OutputStream* os,
                   const CollocatedRequestHandlerPtr& handler,
                   Int requestId,
                   Int batchRequestNum) :
        _outAsync(outAsync), _os(os), _handler(handler), _requestId(requestId), _batchRequestNum(batchRequestNum)
    {
    }

    virtual void
    run()
    {
        if(_handler->sentAsync(_outAsync.get()))
        {
            _handler->invokeAll(_os, _requestId, _batchRequestNum);
        }
    }

private:

    OutgoingAsyncBasePtr _outAsync;
    OutputStream* _os;
    CollocatedRequestHandlerPtr _handler;
    Int _requestId;
    Int _batchRequestNum;
};

void
fillInValue(OutputStream* os, int pos, Int value)
{
    const Byte* p = reinterpret_cast<const Byte*>(&value);
#ifdef ICE_BIG_ENDIAN
    reverse_copy(p, p + sizeof(Int), os->b.begin() + pos);
#else
    copy(p, p + sizeof(Int), os->b.begin() + pos);
#endif
}

}

CollocatedRequestHandler::CollocatedRequestHandler(const ReferencePtr& ref, const ObjectAdapterPtr& adapter) :
    RequestHandler(ref),
    _adapter(ICE_DYNAMIC_CAST(ObjectAdapterI, adapter)),
    _dispatcher(_reference->getInstance()->initializationData().dispatcher),
    _logger(_reference->getInstance()->initializationData().logger), // Cached for better performance.
    _traceLevels(_reference->getInstance()->traceLevels()), // Cached for better performance.
    _requestId(0)
{
}

CollocatedRequestHandler::~CollocatedRequestHandler()
{
}

RequestHandlerPtr
CollocatedRequestHandler::update(const RequestHandlerPtr& previousHandler, const RequestHandlerPtr& newHandler)
{
    return previousHandler.get() == this ? newHandler : ICE_SHARED_FROM_THIS;
}

AsyncStatus
CollocatedRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& outAsync)
{
    return outAsync->invokeCollocated(this);
}

void
CollocatedRequestHandler::asyncRequestCanceled(const OutgoingAsyncBasePtr& outAsync, const LocalException& ex)
{
    Lock sync(*this);

    map<OutgoingAsyncBasePtr, Int>::iterator p = _sendAsyncRequests.find(outAsync);
    if(p != _sendAsyncRequests.end())
    {
        if(p->second > 0)
        {
            _asyncRequests.erase(p->second);
        }
        _sendAsyncRequests.erase(p);
        if(outAsync->exception(ex))
        {
            outAsync->invokeExceptionAsync();
        }
        _adapter->decDirectCount(); // invokeAll won't be called, decrease the direct count.
        return;
    }

    OutgoingAsyncPtr o = ICE_DYNAMIC_CAST(OutgoingAsync, outAsync);
    if(o)
    {
        for(map<Int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
        {
            if(q->second.get() == o.get())
            {
                _asyncRequests.erase(q);
                if(outAsync->exception(ex))
                {
                    outAsync->invokeExceptionAsync();
                }
                return;
            }
        }
    }
}

AsyncStatus
CollocatedRequestHandler::invokeAsyncRequest(OutgoingAsyncBase* outAsync, int batchRequestNum, bool synchronous)
{
    //
    // Increase the direct count to prevent the thread pool from being destroyed before
    // invokeAll is called. This will also throw if the object adapter has been deactivated.
    //
    _adapter->incDirectCount();

    int requestId = 0;
    try
    {
        Lock sync(*this);

        //
        // This will throw if the request is canceled
        //
        outAsync->cancelable(ICE_SHARED_FROM_THIS);

        if(_response)
        {
            requestId = ++_requestId;
            _asyncRequests.insert(make_pair(requestId, ICE_GET_SHARED_FROM_THIS(outAsync)));
        }

        _sendAsyncRequests.insert(make_pair(ICE_GET_SHARED_FROM_THIS(outAsync), requestId));
    }
    catch(...)
    {
         _adapter->decDirectCount();
         throw;
    }

    outAsync->attachCollocatedObserver(_adapter, requestId);

    if(!synchronous || !_response || _reference->getInvocationTimeout() > 0)
    {
        // Don't invoke from the user thread if async or invocation timeout is set
            _adapter->getThreadPool()->dispatch(new InvokeAllAsync(ICE_GET_SHARED_FROM_THIS(outAsync),
                                                                   outAsync->getOs(),
                                                                   ICE_SHARED_FROM_THIS,
                                                                   requestId,
                                                                   batchRequestNum));
    }
    else if(_dispatcher)
    {
        _adapter->getThreadPool()->dispatchFromThisThread(new InvokeAllAsync(ICE_GET_SHARED_FROM_THIS(outAsync),
                                                                             outAsync->getOs(),
                                                                             ICE_SHARED_FROM_THIS,
                                                                             requestId,
                                                                             batchRequestNum));
    }
    else // Optimization: directly call invokeAll if there's no dispatcher.
    {
        //
        // Make sure to hold a reference on this handler while the call is being
        // dispatched. Otherwise, the handler could be deleted during the dispatch
        // if a retry occurs.
        //

        CollocatedRequestHandlerPtr self(ICE_SHARED_FROM_THIS);
        if(sentAsync(outAsync))
        {
            invokeAll(outAsync->getOs(), requestId, batchRequestNum);
        }
    }
    return AsyncStatusQueued;
}

void
CollocatedRequestHandler::sendResponse(Int requestId, OutputStream* os, Byte, bool amd)
{
    OutgoingAsyncBasePtr outAsync;
    {
        Lock sync(*this);
        assert(_response);

        if(_traceLevels->protocol >= 1)
        {
            fillInValue(os, 10, static_cast<Int>(os->b.size()));
        }

        InputStream is(os->instance(), os->getEncoding(), *os, true); // Adopting the OutputStream's buffer.
        is.pos(sizeof(replyHdr) + 4);

        if(_traceLevels->protocol >= 1)
        {
            traceRecv(is, _logger, _traceLevels);
        }

        map<int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.find(requestId);
        if(q != _asyncRequests.end())
        {
            is.swap(*q->second->getIs());
            if(q->second->response())
            {
                outAsync = q->second;
            }
            _asyncRequests.erase(q);
        }
    }

    if(outAsync)
    {
        //
        // If called from an AMD dispatch, invoke asynchronously
        // the completion callback since this might be called from
        // the user code.
        //
        if(amd)
        {
            outAsync->invokeResponseAsync();
        }
        else
        {
            outAsync->invokeResponse();
        }
    }

    _adapter->decDirectCount();
}

void
CollocatedRequestHandler::sendNoResponse()
{
    _adapter->decDirectCount();
}

bool
CollocatedRequestHandler::systemException(Int requestId, const SystemException& ex, bool amd)
{
    handleException(requestId, ex, amd);
    _adapter->decDirectCount();
    return true;
}

void
CollocatedRequestHandler::invokeException(Int requestId, const LocalException& ex, int invokeNum, bool amd)
{
    handleException(requestId, ex, amd);
    _adapter->decDirectCount();
}

ConnectionIPtr
CollocatedRequestHandler::getConnection()
{
    return 0;
}

ConnectionIPtr
CollocatedRequestHandler::waitForConnection()
{
    return 0;
}

bool
CollocatedRequestHandler::sentAsync(OutgoingAsyncBase* outAsync)
{
    {
        Lock sync(*this);
        if(_sendAsyncRequests.erase(ICE_GET_SHARED_FROM_THIS(outAsync)) == 0)
        {
            return false; // The request timed-out.
        }

        if(!outAsync->sent())
        {
            return true;
        }
    }
    outAsync->invokeSent();
    return true;
}

void
CollocatedRequestHandler::invokeAll(OutputStream* os, Int requestId, Int batchRequestNum)
{
    if(_traceLevels->protocol >= 1)
    {
        fillInValue(os, 10, static_cast<Int>(os->b.size()));
        if(requestId > 0)
        {
            fillInValue(os, headerSize, requestId);
        }
        else if(batchRequestNum > 0)
        {
            fillInValue(os, headerSize, batchRequestNum);
        }
        traceSend(*os, _logger, _traceLevels);
    }

    InputStream is(os->instance(), os->getEncoding(), *os);

    if(batchRequestNum > 0)
    {
        is.pos(sizeof(requestBatchHdr));
    }
    else
    {
        is.pos(sizeof(requestHdr));
    }

    int invokeNum = batchRequestNum > 0 ? batchRequestNum : 1;
    ServantManagerPtr servantManager = _adapter->getServantManager();
    try
    {
        while(invokeNum > 0)
        {
            //
            // Increase the direct count for the dispatch. We increase it again here for
            // each dispatch. It's important for the direct count to be > 0 until the last
            // collocated request response is sent to make sure the thread pool isn't
            // destroyed before.
            //
            try
            {
                _adapter->incDirectCount();
            }
            catch(const ObjectAdapterDeactivatedException& ex)
            {
                handleException(requestId, ex, false);
                break;
            }

            Incoming in(_reference->getInstance().get(), this, 0, _adapter, _response, 0, requestId);
            in.invoke(servantManager, &is);
            --invokeNum;
        }
    }
    catch(const LocalException& ex)
    {
        invokeException(requestId, ex, invokeNum, false); // Fatal invocation exception
    }

    _adapter->decDirectCount();
}

void
CollocatedRequestHandler::handleException(int requestId, const Exception& ex, bool amd)
{
    if(requestId == 0)
    {
        return; // Ignore exception for oneway messages.
    }

    OutgoingAsyncBasePtr outAsync;
    {
        Lock sync(*this);

        map<int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.find(requestId);
        if(q != _asyncRequests.end())
        {
            if(q->second->exception(ex))
            {
                outAsync = q->second;
            }
            _asyncRequests.erase(q);
        }
    }

    if(outAsync)
    {
        //
        // If called from an AMD dispatch, invoke asynchronously
        // the completion callback since this might be called from
        // the user code.
        //
        if(amd)
        {
            outAsync->invokeExceptionAsync();
        }
        else
        {
            outAsync->invokeException();
        }
    }
}
