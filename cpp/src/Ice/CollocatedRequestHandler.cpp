// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

#include <Ice/TraceUtil.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{

class InvokeAll : public DispatchWorkItem
{
public:
        
    InvokeAll(OutgoingMessageCallback* out,
              BasicStream* os,
              CollocatedRequestHandler* handler, 
              Int requestId,
              Int invokeNum, 
              bool batch) : 
        _out(out), _os(os), _handler(handler), _requestId(requestId), _invokeNum(invokeNum), _batch(batch)
    {
    }

    virtual void
    run()
    {
        if(_handler->sent(_out))
        {
            _handler->invokeAll(_os, _requestId, _invokeNum, _batch);
        }
    }
    
private:
    
    OutgoingMessageCallback* _out;
    BasicStream* _os;
    CollocatedRequestHandlerPtr _handler;
    Int _requestId;
    Int _invokeNum;
    bool _batch;
};

class InvokeAllAsync : public DispatchWorkItem
{
public:
        
    InvokeAllAsync(const OutgoingAsyncMessageCallbackPtr& outAsync,
                   BasicStream* os,
                   CollocatedRequestHandler* handler, 
                   Int requestId,
                   Int invokeNum, 
                   bool batch) : 
        _outAsync(outAsync), _os(os), _handler(handler), _requestId(requestId), _invokeNum(invokeNum), _batch(batch)
    {
    }

    virtual void
    run()
    {
        if(_handler->sentAsync(_outAsync.get()))
        {
            _handler->invokeAll(_os, _requestId, _invokeNum, _batch);
        }
    }
    
private:
    
    OutgoingAsyncMessageCallbackPtr _outAsync;
    BasicStream* _os;
    CollocatedRequestHandlerPtr _handler;
    Int _requestId;
    Int _invokeNum;
    bool _batch;
};

class InvokeBatchRequests : public DispatchWorkItem
{
public:
    
    InvokeBatchRequests(const CollocatedRequestHandlerPtr& handler,
                        BasicStream& stream,
                        Int invokeNum) :
        _handler(handler),
        _stream(stream.instance(), currentProtocolEncoding),
        _invokeNum(invokeNum)
    {
        _stream.swap(stream);
    }
    
    virtual void
    run()
    {
        _handler->invokeAll(&_stream, 0, _invokeNum, true);
    }
    
private:
    
    const CollocatedRequestHandlerPtr _handler;
    const OutgoingAsyncMessageCallbackPtr _outAsync;
    BasicStream _stream;
    Int _invokeNum;
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
    _batchAutoFlush(
        ref->getInstance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
    _requestId(0),
    _batchStreamInUse(false),
    _batchRequestNum(0),
    _batchStream(ref->getInstance().get(), currentProtocolEncoding, _batchAutoFlush)
{
}

CollocatedRequestHandler::~CollocatedRequestHandler()
{
}

void
CollocatedRequestHandler::prepareBatchRequest(BasicStream* os)
{
    Lock sync(*this);
    while(_batchStreamInUse)
    {
        wait();
    }

    if(_batchStream.b.empty())
    {
        try
        {
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
        }
        catch(const LocalException& ex)
        {
            ex.ice_throw();
        }
    }

    _batchStreamInUse = true;
    _batchMarker = _batchStream.b.size();
    _batchStream.swap(*os);
}

void
CollocatedRequestHandler::finishBatchRequest(BasicStream* os)
{
    try
    {
        Lock sync(*this);
        _batchStream.swap(*os);

        if(_batchAutoFlush && (_batchStream.b.size() > _reference->getInstance()->messageSizeMax()))
        {
            //
            // Temporarily save the last request.
            //
            vector<Byte> lastRequest(_batchStream.b.begin() + _batchMarker, _batchStream.b.end());
            _batchStream.b.resize(_batchMarker);

            _adapter->getThreadPool()->dispatch(new InvokeBatchRequests(this, _batchStream, _batchRequestNum));
            
            //
            // Reset the batch.
            //
            BasicStream dummy(_reference->getInstance().get(), currentProtocolEncoding, _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestNum = 0;
            _batchMarker = 0;
            
            //
            // Check again if the last request doesn't exceed what we can send with the auto flush
            //
            if(sizeof(requestBatchHdr) + lastRequest.size() >  _reference->getInstance()->messageSizeMax())
            {
                Ex::throwMemoryLimitException(__FILE__, __LINE__, sizeof(requestBatchHdr) + lastRequest.size(),
                                              _reference->getInstance()->messageSizeMax());
            }
            
            //
            // Start a new batch with the last message that caused us to go over the limit.
            //
            _batchStream.writeBlob(requestBatchHdr, sizeof(requestBatchHdr));
            _batchStream.writeBlob(&lastRequest[0], lastRequest.size());
        }
        
        //
        // Increment the number of requests in the batch.
        //
        assert(_batchStreamInUse);
        ++_batchRequestNum;
        _batchStreamInUse = false;
        notifyAll();
    }
    catch(const LocalException&)
    {
        abortBatchRequest();
        throw;
    }
}

void
CollocatedRequestHandler::abortBatchRequest()
{
    Lock sync(*this);

    BasicStream dummy(_reference->getInstance().get(), currentProtocolEncoding, _batchAutoFlush);
    _batchStream.swap(dummy);
    _batchRequestNum = 0;
    _batchMarker = 0;

    assert(_batchStreamInUse);
    _batchStreamInUse = false;
    notifyAll();
}

bool
CollocatedRequestHandler::sendRequest(OutgoingMessageCallback* out)
{
    out->invokeCollocated(this);
    return !_response && _reference->getInvocationTimeout() == 0;
}

AsyncStatus
CollocatedRequestHandler::sendAsyncRequest(const OutgoingAsyncMessageCallbackPtr& outAsync)
{
    return outAsync->__invokeCollocated(this);
}

void 
CollocatedRequestHandler::requestTimedOut(OutgoingMessageCallback* out)
{
    Lock sync(*this);
    
    map<OutgoingMessageCallback*, Int>::iterator p = _sendRequests.find(out);
    if(p != _sendRequests.end())
    {
        if(p->second > 0)
        {
            _requests.erase(p->second);
        }
        InvocationTimeoutException ex(__FILE__, __LINE__);
        out->finished(ex);
        _sendRequests.erase(p);
        return;
    }

    Outgoing* o = dynamic_cast<Outgoing*>(out);
    if(o)
    {
        for(map<Int, Outgoing*>::iterator q = _requests.begin(); q != _requests.end(); ++q)
        {
            if(q->second == o)
            {
                InvocationTimeoutException ex(__FILE__, __LINE__);
                o->finished(ex);
                _requests.erase(q);
                return; // We're done.
            }
        }
    }
}

void 
CollocatedRequestHandler::asyncRequestTimedOut(const OutgoingAsyncMessageCallbackPtr& outAsync)
{
    Lock sync(*this);
    
    map<OutgoingAsyncMessageCallbackPtr, Int>::iterator p = _sendAsyncRequests.find(outAsync);
    if(p != _sendAsyncRequests.end())
    {
        if(p->second > 0)
        {
            _asyncRequests.erase(p->second);
        }
        _sendAsyncRequests.erase(p);
        outAsync->__dispatchInvocationTimeout(_reference->getInstance()->clientThreadPool(), 0);
        return;
    }

    OutgoingAsyncPtr o = OutgoingAsyncPtr::dynamicCast(outAsync);
    if(o)
    {
        for(map<Int, OutgoingAsyncPtr>::iterator q = _asyncRequests.begin(); q != _asyncRequests.end(); ++q)
        {
            if(q->second.get() == o.get())
            {
                _asyncRequests.erase(q);
                outAsync->__dispatchInvocationTimeout(_reference->getInstance()->clientThreadPool(), 0);
                return;
            }
        }
    }
}

void
CollocatedRequestHandler::invokeRequest(Outgoing* out)
{
    int requestId = 0;
    if(_reference->getInvocationTimeout() > 0 || _response)
    {
        Lock sync(*this);
        if(_response)
        {
            requestId = ++_requestId;
            _requests.insert(make_pair(requestId, out));
        }
        if(_reference->getInvocationTimeout() > 0)
        {
            _sendRequests.insert(make_pair(out, requestId));
        }
    } 

    out->attachCollocatedObserver(_adapter, requestId);

    if(_reference->getInvocationTimeout() > 0)
    {
        // Don't invoke from the user thread, invocation timeouts wouldn't work otherwise.
        _adapter->getThreadPool()->dispatch(new InvokeAll(out, out->os(), this, requestId, 1, false));
    }
    else if(_dispatcher)
    {
        _adapter->getThreadPool()->dispatchFromThisThread(new InvokeAll(out, out->os(), this, requestId, 1, false));
    }
    else // Optimization: directly call invokeAll if there's no dispatcher.
    {
        out->sent();
        invokeAll(out->os(), requestId, 1, false);
    }
}

AsyncStatus
CollocatedRequestHandler::invokeAsyncRequest(OutgoingAsync* outAsync)
{
    int requestId = 0;
    if(_reference->getInvocationTimeout() > 0 || _response)
    {
        Lock sync(*this);
        if(_response)
        {
            requestId = ++_requestId;
            _asyncRequests.insert(make_pair(requestId, outAsync));
        }
        if(_reference->getInvocationTimeout() > 0)
        {
            _sendAsyncRequests.insert(make_pair(outAsync, requestId));
        }
    }

    outAsync->__attachCollocatedObserver(_adapter, requestId);

    _adapter->getThreadPool()->dispatch(new InvokeAllAsync(outAsync, outAsync->__getOs(), this, requestId, 1, false));
    return AsyncStatusQueued;
}

void
CollocatedRequestHandler::invokeBatchRequests(BatchOutgoing* out)
{
    Int invokeNum;
    {
        Lock sync(*this);
        while(_batchStreamInUse)
        {
            wait();
        }
        
        invokeNum = _batchRequestNum;

        if(_batchRequestNum > 0)
        {
            if(_reference->getInvocationTimeout() > 0)
            {
                _sendRequests.insert(make_pair(out, 0));
            }

            assert(!_batchStream.b.empty());
            _batchStream.swap(*out->os());
            
            //
            // Reset the batch stream.
            //
            BasicStream dummy(_reference->getInstance().get(), currentProtocolEncoding, _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestNum = 0;
            _batchMarker = 0;
        }
    }
        
    out->attachCollocatedObserver(_adapter, 0);

    if(invokeNum > 0)
    {
        if(_reference->getInvocationTimeout() > 0)
        {
            _adapter->getThreadPool()->dispatch(new InvokeAll(out, out->os(), this, 0, invokeNum, true));
        }
        else if(_dispatcher)
        {
            _adapter->getThreadPool()->dispatchFromThisThread(new InvokeAll(out, out->os(), this, 0, invokeNum, true));
        }
        else // Optimization: directly call invokeAll if there's no dispatcher.
        {
            out->sent();
            invokeAll(out->os(), 0, invokeNum, true); // Invoke from the user thread.
        }
    }
    else
    {
        out->sent();
    }
}

AsyncStatus
CollocatedRequestHandler::invokeAsyncBatchRequests(BatchOutgoingAsync* outAsync)
{
    Int invokeNum;
    {
        Lock sync(*this);
        while(_batchStreamInUse)
        {
            wait();
        }
        
        invokeNum = _batchRequestNum;
        if(_batchRequestNum > 0)
        {
            if(_reference->getInvocationTimeout() > 0)
            {
                _sendAsyncRequests.insert(make_pair(outAsync, 0));
            }

            assert(!_batchStream.b.empty());
            _batchStream.swap(*outAsync->__getOs());
            
            //
            // Reset the batch stream.
            //
            BasicStream dummy(_reference->getInstance().get(), currentProtocolEncoding, _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestNum = 0;
            _batchMarker = 0;
        }
    }

    outAsync->__attachCollocatedObserver(_adapter, 0);
    
    if(invokeNum > 0)
    {
        _adapter->getThreadPool()->dispatch(new InvokeAllAsync(outAsync, outAsync->__getOs(), this, 0, invokeNum,true));
        return AsyncStatusQueued;
    }
    else if(outAsync->__sent())
    {
        return static_cast<AsyncStatus>(AsyncStatusSent | AsyncStatusInvokeSentCallback);
    }
    else
    {
        return AsyncStatusSent;
    }
}

void 
CollocatedRequestHandler::sendResponse(Int requestId, BasicStream* os, Byte)
{
    OutgoingAsyncPtr outAsync;
    {
        Lock sync(*this);
        assert(_response);

        os->i = os->b.begin() + sizeof(replyHdr) + 4;

        if(_traceLevels->protocol >= 1)
        {
            fillInValue(os, 10, static_cast<Int>(os->b.size()));
            traceRecv(*os, _logger, _traceLevels);
        }

        map<int, Outgoing*>::iterator p = _requests.find(requestId);
        if(p != _requests.end())
        {
            p->second->finished(*os);
            _requests.erase(p);
        }
        else
        {
            map<int, OutgoingAsyncPtr>::iterator q = _asyncRequests.find(requestId);
            if(q != _asyncRequests.end())
            {
                os->swap(*q->second->__getIs());
                outAsync = q->second;
                _asyncRequests.erase(q);
            }
        }
    }

    if(outAsync && outAsync->__finished())
    {
        outAsync->__invokeCompleted();
    }
    _adapter->decDirectCount();
}

void
CollocatedRequestHandler::sendNoResponse()
{
    _adapter->decDirectCount();
}

bool
CollocatedRequestHandler::systemException(Int requestId, const SystemException& ex)
{
    handleException(requestId, ex);
    _adapter->decDirectCount();
    return true;
}

void 
CollocatedRequestHandler::invokeException(Int requestId, const LocalException& ex, int invokeNum)
{
    if(requestId > 0)
    {
        Lock sync(*this);
        _requests.erase(requestId);
        _asyncRequests.erase(requestId);
    }
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
CollocatedRequestHandler::sent(OutgoingMessageCallback* out)
{
    if(_reference->getInvocationTimeout() > 0)
    {
        Lock sync(*this);
        if(_sendRequests.erase(out) == 0)
        {
            return false; // The request timed-out.
        }
    }
    out->sent();
    return true;
}

bool
CollocatedRequestHandler::sentAsync(OutgoingAsyncMessageCallback* outAsync)
{
    if(_reference->getInvocationTimeout() > 0)
    {
        Lock sync(*this);
        if(_sendAsyncRequests.erase(outAsync) == 0)
        {
            return false; // The request timed-out.
        }
    }
    if(outAsync->__sent())
    {
        outAsync->__invokeSent();
    }
    return true;
}

void
CollocatedRequestHandler::invokeAll(BasicStream* os, Int requestId, Int invokeNum, bool batch)
{
    if(batch)
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
        else if(batch)
        {
            fillInValue(os, headerSize, invokeNum);
        }
        traceSend(*os, _logger, _traceLevels);
    }

    ServantManagerPtr servantManager = _adapter->getServantManager();
    try
    {
        while(invokeNum > 0)
        {        
            try
            {
                _adapter->incDirectCount();
            }
            catch(const ObjectAdapterDeactivatedException& ex)
            {
                handleException(requestId, ex);
                return;
            }

            Incoming in(_reference->getInstance().get(), this, 0, _adapter, _response, 0, requestId);
            in.invoke(servantManager, os);
            --invokeNum;
        }
    }
    catch(const LocalException& ex)
    {
        invokeException(requestId, ex, invokeNum); // Fatal invocation exception
    }
}

void
CollocatedRequestHandler::handleException(int requestId, const Exception& ex)
{
    if(requestId == 0)
    {
        return; // Ignore exception for oneway messages.
    }

    OutgoingAsyncPtr outAsync;
    {
        Lock sync(*this);
        
        map<int, Outgoing*>::iterator p = _requests.find(requestId);
        if(p != _requests.end())
        {
            p->second->finished(ex);
            _requests.erase(p);
        }
        else
        {
            map<int, OutgoingAsyncPtr>::iterator q = _asyncRequests.find(requestId);
            if(q != _asyncRequests.end())
            {
                outAsync = q->second;
                _asyncRequests.erase(q);
            }
        }
    }
    if(outAsync)
    {
        outAsync->__finished(ex);
    }
}
