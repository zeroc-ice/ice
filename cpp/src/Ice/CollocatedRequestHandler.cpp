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
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>

#include <Ice/TraceUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class InvokeAll : public DispatchWorkItem
{
public:

    InvokeAll(OutgoingBase* out,
              BasicStream* os,
              CollocatedRequestHandler* handler,
              Int requestId,
              Int batchRequestNum) :
        _out(out), _os(os), _handler(handler), _requestId(requestId), _batchRequestNum(batchRequestNum)
    {
    }

    virtual void
    run()
    {
        if(_handler->sent(_out))
        {
            _handler->invokeAll(_os, _requestId, _batchRequestNum);
        }
    }

private:

    OutgoingBase* _out;
    BasicStream* _os;
    CollocatedRequestHandlerPtr _handler;
    Int _requestId;
    Int _batchRequestNum;
};

class InvokeAllAsync : public DispatchWorkItem
{
public:

    InvokeAllAsync(const OutgoingAsyncBasePtr& outAsync,
                   BasicStream* os,
                   CollocatedRequestHandler* handler,
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
    BasicStream* _os;
    CollocatedRequestHandlerPtr _handler;
    Int _requestId;
    Int _batchRequestNum;
};

void
fillInValue(BasicStream* os, int pos, Int value)
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
    _adapter(ObjectAdapterIPtr::dynamicCast(adapter)),
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
    return previousHandler.get() == this ? newHandler : this;
}

bool
CollocatedRequestHandler::sendRequest(ProxyOutgoingBase* out)
{
    out->invokeCollocated(this);
    return !_response && _reference->getInvocationTimeout() == 0;
}

AsyncStatus
CollocatedRequestHandler::sendAsyncRequest(const ProxyOutgoingAsyncBasePtr& outAsync)
{
    return outAsync->invokeCollocated(this);
}

void
CollocatedRequestHandler::requestCanceled(OutgoingBase* out, const LocalException& ex)
{
    Lock sync(*this);

    map<OutgoingBase*, Int>::iterator p = _sendRequests.find(out);
    if(p != _sendRequests.end())
    {
        if(p->second > 0)
        {
            _requests.erase(p->second);
        }
        InvocationTimeoutException ex(__FILE__, __LINE__);
        out->completed(ex);
        _sendRequests.erase(p);
        _adapter->decDirectCount(); // invokeAll won't be called, decrease the direct count.
        return;
    }

    Outgoing* o = dynamic_cast<Outgoing*>(out);
    if(o)
    {
        for(map<Int, OutgoingBase*>::iterator q = _requests.begin(); q != _requests.end(); ++q)
        {
            if(q->second == o)
            {
                InvocationTimeoutException ex(__FILE__, __LINE__);
                q->second->completed(ex);
                _requests.erase(q);
                return; // We're done.
            }
        }
    }
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
        if(outAsync->completed(ex))
        {
            outAsync->invokeCompletedAsync();
        }
        _adapter->decDirectCount(); // invokeAll won't be called, decrease the direct count.
        return;
    }

    OutgoingAsyncPtr o = OutgoingAsyncPtr::dynamicCast(outAsync);
    if(o)
    {
        for(map<Int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
        {
            if(q->second.get() == o.get())
            {
                _asyncRequests.erase(q);
                if(outAsync->completed(ex))
                {
                    outAsync->invokeCompletedAsync();
                }
                return;
            }
        }
    }
}

void
CollocatedRequestHandler::invokeRequest(OutgoingBase* out, int batchRequestNum)
{
    //
    // Increase the direct count to prevent the thread pool from being destroyed before
    // invokeAll is called. This will also throw if the object adapter has been deactivated.
    //
    _adapter->incDirectCount();

    int requestId = 0;
    {
        Lock sync(*this);
        if(_response)
        {
            requestId = ++_requestId;
            _requests.insert(make_pair(requestId, out));
        }

        _sendRequests.insert(make_pair(out, requestId));
    }

    out->attachCollocatedObserver(_adapter, requestId);

    if(_reference->getInvocationTimeout() > 0)
    {
        // Don't invoke from the user thread, invocation timeouts wouldn't work otherwise.
        _adapter->getThreadPool()->dispatch(new InvokeAll(out, out->os(), this, requestId, batchRequestNum));
    }
    else if(_dispatcher)
    {
        _adapter->getThreadPool()->dispatchFromThisThread(new InvokeAll(out, out->os(), this, requestId,
                                                                        batchRequestNum));
    }
    else // Optimization: directly call invokeAll if there's no dispatcher.
    {
        out->sent();
        invokeAll(out->os(), requestId, batchRequestNum);
    }
}

AsyncStatus
CollocatedRequestHandler::invokeAsyncRequest(OutgoingAsyncBase* outAsync, int batchRequestNum)
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

        outAsync->cancelable(this); // This will throw if the request is canceled

        if(_response)
        {
            requestId = ++_requestId;
            _asyncRequests.insert(make_pair(requestId, outAsync));
        }

        _sendAsyncRequests.insert(make_pair(outAsync, requestId));
    }
    catch(...)
    {
        _adapter->decDirectCount();
        throw;
    }

    outAsync->attachCollocatedObserver(_adapter, requestId);

    _adapter->getThreadPool()->dispatch(new InvokeAllAsync(outAsync, outAsync->getOs(), this, requestId,
                                                           batchRequestNum));
    return AsyncStatusQueued;
}

void
CollocatedRequestHandler::sendResponse(Int requestId, BasicStream* os, Byte, bool amd)
{
    OutgoingAsyncBasePtr outAsync;
    {
        Lock sync(*this);
        assert(_response);

        os->i = os->b.begin() + sizeof(replyHdr) + 4;

        if(_traceLevels->protocol >= 1)
        {
            fillInValue(os, 10, static_cast<Int>(os->b.size()));
            traceRecv(*os, _logger, _traceLevels);
        }

        map<int, OutgoingBase*>::iterator p = _requests.find(requestId);
        if(p != _requests.end())
        {
            p->second->completed(*os);
            _requests.erase(p);
        }
        else
        {
            map<int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.find(requestId);
            if(q != _asyncRequests.end())
            {
                os->swap(*q->second->getIs());
                if(q->second->completed())
                {
                    outAsync = q->second;
                }
                _asyncRequests.erase(q);
            }
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
            outAsync->invokeCompletedAsync();
        }
        else
        {
            outAsync->invokeCompleted();
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
CollocatedRequestHandler::sent(OutgoingBase* out)
{
    Lock sync(*this);
    if(_sendRequests.erase(out) == 0)
    {
        return false; // The request timed-out.
    }
    out->sent();
    return true;
}

bool
CollocatedRequestHandler::sentAsync(OutgoingAsyncBase* outAsync)
{
    {
        Lock sync(*this);
        if(_sendAsyncRequests.erase(outAsync) == 0)
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
CollocatedRequestHandler::invokeAll(BasicStream* os, Int requestId, Int batchRequestNum)
{
    if(batchRequestNum > 0)
    {
        os->i = os->b.begin() + sizeof(requestBatchHdr);
    }
    else
    {
        os->i = os->b.begin() + sizeof(requestHdr);
    }

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
            in.invoke(servantManager, os);
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

        map<int, OutgoingBase*>::iterator p = _requests.find(requestId);
        if(p != _requests.end())
        {
            p->second->completed(ex);
            _requests.erase(p);
        }
        else
        {
            map<int, OutgoingAsyncBasePtr>::iterator q = _asyncRequests.find(requestId);
            if(q != _asyncRequests.end())
            {
                if(q->second->completed(ex))
                {
                    outAsync = q->second;
                }
                _asyncRequests.erase(q);
            }
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
            outAsync->invokeCompletedAsync();
        }
        else
        {
            outAsync->invokeCompleted();
        }
    }
}
