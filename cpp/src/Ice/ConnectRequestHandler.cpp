// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/ConnectRequestHandler.h>
#include <Ice/ConnectionRequestHandler.h>
#include <Ice/Instance.h>
#include <Ice/Proxy.h>
#include <Ice/ConnectionI.h>
#include <Ice/RouterInfo.h>
#include <Ice/Outgoing.h>
#include <Ice/OutgoingAsync.h>
#include <Ice/Protocol.h>
#include <Ice/Properties.h>
#include <Ice/ThreadPool.h>

using namespace std;
using namespace IceInternal;

namespace
{

class FlushRequestsWithException : public DispatchWorkItem
{
public:
    
    FlushRequestsWithException(const InstancePtr& instance,
                               const ConnectRequestHandlerPtr& handler, 
                               const Ice::LocalException& ex) :
        DispatchWorkItem(instance),
        _handler(handler),
        _exception(ex.ice_clone())
    {
    }
    
    virtual void
    run()
    {
        _handler->flushRequestsWithException(*_exception.get());
    }
    
private:
    
    const ConnectRequestHandlerPtr _handler;
    const IceUtil::UniquePtr<Ice::LocalException> _exception;
};

class FlushRequestsWithExceptionWrapper : public DispatchWorkItem
{
public:
    
    FlushRequestsWithExceptionWrapper(const InstancePtr& instance,
                                      const ConnectRequestHandlerPtr& handler, 
                                      const LocalExceptionWrapper& ex) :
        DispatchWorkItem(instance),
        _handler(handler),
        _exception(ex)
    {
    }
    
    virtual void
    run()
    {
        _handler->flushRequestsWithException(_exception);
    }
    
private:
    
    const ConnectRequestHandlerPtr _handler;
    const LocalExceptionWrapper _exception;
};

class FlushSentRequests : public DispatchWorkItem
{
public:
    
    FlushSentRequests(const InstancePtr& instance, const vector<OutgoingAsyncMessageCallbackPtr>& callbacks) :
        DispatchWorkItem(instance), _callbacks(callbacks)
    {
    }

    virtual void
    run()
    {
        for(vector<OutgoingAsyncMessageCallbackPtr>::const_iterator p = _callbacks.begin(); p != _callbacks.end(); ++p)
        {
            (*p)->__sent();
        }
    }

private:

    vector<OutgoingAsyncMessageCallbackPtr> _callbacks;
};

};

ConnectRequestHandler::ConnectRequestHandler(const ReferencePtr& ref, 
                                             const Ice::ObjectPrx& proxy,
                                             const Handle< ::IceDelegate::Ice::Object>& delegate) :
    RequestHandler(ref),
    _proxy(proxy),
    _delegate(delegate),
    _batchAutoFlush(
        ref->getInstance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
    _initialized(false),
    _flushing(false),
    _batchRequestInProgress(false),
    _batchRequestsSize(sizeof(requestBatchHdr)),
    _batchStream(ref->getInstance().get(), Ice::currentProtocolEncoding, _batchAutoFlush),
    _updateRequestHandler(false)
{
}

ConnectRequestHandler::~ConnectRequestHandler()
{
}

RequestHandlerPtr
ConnectRequestHandler::connect()
{
    _reference->getConnection(this);

    Lock sync(*this);
    if(initialized())
    {
        assert(_connection);
        return new ConnectionRequestHandler(_reference, _connection, _compress);
    }
    else
    {
        _updateRequestHandler = true; // The proxy request handler will be updated when the connection is set.
        return this;
    }
}

void
ConnectRequestHandler::prepareBatchRequest(BasicStream* os)
{
    {
        Lock sync(*this);
        while(_batchRequestInProgress)
        {
            wait();
        }

        if(!initialized())
        {
            _batchRequestInProgress = true;
            _batchStream.swap(*os);
            return;
        }
    }
    _connection->prepareBatchRequest(os);
}

void
ConnectRequestHandler::finishBatchRequest(BasicStream* os)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            assert(_batchRequestInProgress);
            _batchRequestInProgress = false;
            notifyAll();

            _batchStream.swap(*os);

            if(!_batchAutoFlush && 
               _batchStream.b.size() + _batchRequestsSize > _reference->getInstance()->messageSizeMax())
            {
                Ex::throwMemoryLimitException(__FILE__, __LINE__, _batchStream.b.size() + _batchRequestsSize,
                                              _reference->getInstance()->messageSizeMax());
            }

            _batchRequestsSize += _batchStream.b.size();

            Request req;
            req.os = new BasicStream(_reference->getInstance().get(), Ice::currentProtocolEncoding, _batchAutoFlush);
            req.os->swap(_batchStream);
            _requests.push_back(req);
            return;
        }
    }
    _connection->finishBatchRequest(os, _compress);
}

void
ConnectRequestHandler::abortBatchRequest()
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            assert(_batchRequestInProgress);
            _batchRequestInProgress = false;
            notifyAll();

            BasicStream dummy(_reference->getInstance().get(), Ice::currentProtocolEncoding, _batchAutoFlush);
            _batchStream.swap(dummy);
            _batchRequestsSize = sizeof(requestBatchHdr);

            return;
        }
    }
    _connection->abortBatchRequest();
}

Ice::ConnectionI*
ConnectRequestHandler::sendRequest(Outgoing* out)
{
    // Must be called first, _compress might not be initialized before this returns.
    Ice::ConnectionIPtr connection = getConnection(true);
    assert(connection);
    if(!connection->sendRequest(out, _compress, _response) || _response)
    {
        return _connection.get(); // The request hasn't been sent or we're expecting a response.
    }
    else
    {
        return 0; // The request has been sent.
    }
}

AsyncStatus
ConnectRequestHandler::sendAsyncRequest(const OutgoingAsyncPtr& out)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            Request req;
            req.out = out;
            _requests.push_back(req);
            return AsyncStatusQueued;
        }
    }
    return _connection->sendAsyncRequest(out, _compress, _response);
}

bool
ConnectRequestHandler::flushBatchRequests(BatchOutgoing* out)
{
    return getConnection(true)->flushBatchRequests(out);
}

AsyncStatus
ConnectRequestHandler::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& out)
{
    {
        Lock sync(*this);
        if(!initialized())
        {
            Request req;
            req.batchOut = out;
            _requests.push_back(req);
            return AsyncStatusQueued;
        }
    }
    return _connection->flushAsyncBatchRequests(out);
}
    
Ice::ConnectionIPtr
ConnectRequestHandler::getConnection(bool waitInit)
{
    if(waitInit)
    {
        //
        // Wait for the connection establishment to complete or fail.
        //
        Lock sync(*this);
        while(!_initialized && !_exception.get())
        {
            wait();
        }
    }
     
    if(_exception.get())
    {
        _exception->ice_throw();
        return 0; // Keep the compiler happy.
    }
    else
    {
        assert(!waitInit || _initialized);
        return _connection;
    }
}

void
ConnectRequestHandler::setConnection(const Ice::ConnectionIPtr& connection, bool compress)
{
    {
        Lock sync(*this);
        assert(!_exception.get() && !_connection);
        assert(_updateRequestHandler || _requests.empty());

        _connection = connection;
        _compress = compress;
    }
    
    //
    // If this proxy is for a non-local object, and we are using a router, then
    // add this proxy to the router info object.
    //
    RouterInfoPtr ri = _reference->getRouterInfo();
    if(ri && !ri->addProxy(_proxy, this))
    {
        return; // The request handler will be initialized once addProxy returns.
    }

    //
    // We can now send the queued requests.
    //
    flushRequests();
}

void
ConnectRequestHandler::setException(const Ice::LocalException& ex)
{
    Lock sync(*this);
    assert(!_initialized && !_exception.get());
    assert(_updateRequestHandler || _requests.empty());

    _exception.reset(ex.ice_clone());
    _proxy = 0; // Break cyclic reference count.
    _delegate = 0; // Break cyclic reference count.

    //
    // If some requests were queued, we notify them of the failure. This is done from a thread
    // from the client thread pool since this will result in ice_exception callbacks to be 
    // called.
    //
    if(!_requests.empty())
    {
        const InstancePtr instance = _reference->getInstance();
        instance->clientThreadPool()->execute(new FlushRequestsWithException(instance, this, ex));
    }

    notifyAll();
}

void
ConnectRequestHandler::addedProxy()
{
    //
    // The proxy was added to the router info, we're now ready to send the 
    // queued requests.
    //
    flushRequests();
}

bool
ConnectRequestHandler::initialized()
{
    // Must be called with the mutex locked.

    if(_initialized)
    {
        assert(_connection);
        return true;
    }
    else
    {
        while(_flushing && !_exception.get())
        {
            wait();
        }
        
        if(_exception.get())
        {
            _exception->ice_throw();
            return false; // Keep the compiler happy.
        }
        else
        {
            return _initialized;
        }
    }
}

void
ConnectRequestHandler::flushRequests()
{
    {
        Lock sync(*this);
        assert(_connection && !_initialized);
        
        while(_batchRequestInProgress)
        {
            wait();
        }
            
        //
        // We set the _flushing flag to true to prevent any additional queuing. Callers
        // might block for a little while as the queued requests are being sent but this
        // shouldn't be an issue as the request sends are non-blocking.
        // 
        _flushing = true;
    }

    vector<OutgoingAsyncMessageCallbackPtr> sentCallbacks;
    try
    {
        while(!_requests.empty()) // _requests is immutable when _flushing = true
        {
            Request& req = _requests.front();
            if(req.out)
            {
                if(_connection->sendAsyncRequest(req.out, _compress, _response) & AsyncStatusInvokeSentCallback)
                {
                    sentCallbacks.push_back(req.out);
                }
            }
            else if(req.batchOut)
            {
                if(_connection->flushAsyncBatchRequests(req.batchOut) & AsyncStatusInvokeSentCallback)
                {
                    sentCallbacks.push_back(req.batchOut);
                }
            }
            else
            {
                BasicStream os(req.os->instance(), Ice::currentProtocolEncoding);
                _connection->prepareBatchRequest(&os);
                try
                {
                    const Ice::Byte* bytes;
                    req.os->i = req.os->b.begin();
                    req.os->readBlob(bytes, req.os->b.size());
                    os.writeBlob(bytes, req.os->b.size());
                }
                catch(const Ice::LocalException&)
                {
                    _connection->abortBatchRequest();
                    throw;
                }
                _connection->finishBatchRequest(&os, _compress);
                delete req.os;
            }
            _requests.pop_front();
        }
    }
    catch(const LocalExceptionWrapper& ex)
    {
        Lock sync(*this);
        assert(!_exception.get() && !_requests.empty());
        _exception.reset(ex.get()->ice_clone());
        const InstancePtr instance = _reference->getInstance();
        instance->clientThreadPool()->execute(new FlushRequestsWithExceptionWrapper(instance, this, ex));
    }
    catch(const Ice::LocalException& ex)
    {
        Lock sync(*this);
        assert(!_exception.get() && !_requests.empty());
        _exception.reset(ex.ice_clone());
        const InstancePtr instance = _reference->getInstance();
        instance->clientThreadPool()->execute(new FlushRequestsWithException(instance, this, ex));
    }

    if(!sentCallbacks.empty())
    {
        const InstancePtr instance = _reference->getInstance();
        instance->clientThreadPool()->execute(new FlushSentRequests(instance, sentCallbacks));
    }
        
    //
    // We've finished sending the queued requests and the request handler now send
    // the requests over the connection directly. It's time to substitute the 
    // request handler of the proxy with the more efficient connection request 
    // handler which does not have any synchronization. This also breaks the cyclic
    // reference count with the proxy.
    //
    if(_updateRequestHandler && !_exception.get())
    {
        _proxy->__setRequestHandler(_delegate, new ConnectionRequestHandler(_reference, _connection, _compress));
    }

    {
        Lock sync(*this);
        assert(!_initialized);
        if(!_exception.get())
        {
            _initialized = true;
            _flushing = false;
        }
        _proxy = 0; // Break cyclic reference count.
        _delegate = 0; // Break cyclic reference count.
        notifyAll();
    }
}

void
ConnectRequestHandler::flushRequestsWithException(const Ice::LocalException& ex)
{
    for(deque<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(p->out)
        {            
            p->out->__finished(ex, false);
        }
        else if(p->batchOut)
        {
            p->batchOut->__finished(ex, false);
        }
        else
        {
            assert(p->os);
            delete p->os;
        }
    }
    _requests.clear();
}

void
ConnectRequestHandler::flushRequestsWithException(const LocalExceptionWrapper& ex)
{
    for(deque<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(p->out)
        {            
            p->out->__finished(ex);
        }
        else if(p->batchOut)
        {
            p->batchOut->__finished(*ex.get(), false);
        }
        else
        {
            assert(p->os);
            delete p->os;
        }
    }
    _requests.clear();
}
