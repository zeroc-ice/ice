// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

using namespace std;
using namespace IceInternal;

ConnectRequestHandler::ConnectRequestHandler(const ReferencePtr& ref, 
                                             const Ice::ObjectPrx& proxy,
                                             const Handle< ::IceDelegate::Ice::Object>& delegate) :
    RequestHandler(ref),
    _proxy(proxy),
    _delegate(delegate),
    _response(ref->getMode() == Reference::ModeTwoway),
    _batchAutoFlush(
        ref->getInstance()->initializationData().properties->getPropertyAsIntWithDefault("Ice.BatchAutoFlush", 1) > 0),
    _initialized(false),
    _flushing(false),
    _batchRequestInProgress(false),
    _batchRequestsSize(sizeof(requestBatchHdr)),
    _batchStream(ref->getInstance().get(), _batchAutoFlush),
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
    if(_connection)
    {
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
                throw Ice::MemoryLimitException(__FILE__, __LINE__);
            }

            _batchRequestsSize += _batchStream.b.size();

            Request req;
            req.os = new BasicStream(_reference->getInstance().get(), _batchAutoFlush);
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

            BasicStream dummy(_reference->getInstance().get(), _batchAutoFlush);
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
    return (!getConnection(true)->sendRequest(out, _compress, _response) || _response) ? _connection.get() : 0;
}

void
ConnectRequestHandler::sendAsyncRequest(const OutgoingAsyncPtr& out)
{
    try
    {
        {
            Lock sync(*this);
            if(!initialized())
            {
                Request req;
                req.out = out;
                _requests.push_back(req);
                return;
            }
        }
        _connection->sendAsyncRequest(out, _compress, _response);
    }
    catch(const LocalExceptionWrapper& ex)
    {
        out->__finished(ex);
    }
    catch(const Ice::LocalException& ex)
    {
        out->__finished(ex);
    }
}

bool
ConnectRequestHandler::flushBatchRequests(BatchOutgoing* out)
{
    return getConnection(true)->flushBatchRequests(out);
}

void
ConnectRequestHandler::flushAsyncBatchRequests(const BatchOutgoingAsyncPtr& out)
{
    try
    {
        {
            Lock sync(*this);
            if(!initialized())
            {
                Request req;
                req.batchOut = out;
                _requests.push_back(req);
                return;
            }
        }
        _connection->flushAsyncBatchRequests(out);
    }
    catch(const Ice::LocalException& ex)
    {
        out->__finished(ex);
    }
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
        return false; // Keep the compiler happy.
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
        _connection = connection;
        _compress = compress;
    }
    
    //
    // If this proxy is for a non-local object, and we are using a router, then
    // add this proxy to the router info object.
    //
    RouterInfoPtr ri = _reference->getRouterInfo();
    if(ri)
    {
        if(!ri->addProxy(_proxy, this))
        {
            return; // The request handler will be initialized once addProxy returns.
        }
    }
    
    flushRequests();
}

void
ConnectRequestHandler::setException(const Ice::LocalException& ex)
{
    {
        Lock sync(*this);
        _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
        _proxy = 0; // Break cyclic reference count.
        _delegate = 0; // Break cyclic reference count.
        notifyAll();
    }
        
    for(vector<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        if(p->out)
        {            
            p->out->__finished(ex);
        }
        else if(p->batchOut)
        {
            p->batchOut->__finished(ex);
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
ConnectRequestHandler::addedProxy()
{
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
        while(_flushing)
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
        assert(_connection);
        
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
            
    for(vector<Request>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
    {
        // _requests is immutable when _flushing = true
        if(p->out)
        {
            try
            {
                _connection->sendAsyncRequest(p->out, _compress, _response);
            }
            catch(const LocalExceptionWrapper& ex)
            {
                p->out->__finished(ex);
            }
            catch(const Ice::LocalException& ex)
            {
                p->out->__finished(ex);
            }
        }
        else if(p->batchOut)
        {
            try
            {
                _connection->flushAsyncBatchRequests(p->batchOut);
            }
            catch(const Ice::LocalException& ex)
            {
                p->batchOut->__finished(ex);
            }
        }
        else
        {
            assert(p->os);
            if(_exception.get())
            {
                delete p->os;
            }
            else
            {
                //
                // TODO: Add sendBatchRequest() method to ConnectionI?
                //
                try
                {
                    BasicStream os(p->os->instance());
                    _connection->prepareBatchRequest(&os);
                    const Ice::Byte* bytes;
                    p->os->i = p->os->b.begin();
                    p->os->readBlob(bytes, p->os->b.size());
                    os.writeBlob(bytes, p->os->b.size());
                    _connection->finishBatchRequest(&os, _compress);
                    delete p->os;
                }
                catch(const Ice::LocalException& ex)
                {
                    delete p->os;
                    _connection->abortBatchRequest();
                    _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
                }
            }
        }
    }
    _requests.clear();
        
    {
        Lock sync(*this);
        _initialized = true;
        _flushing = false;
        notifyAll();
    }

    if(_updateRequestHandler && !_exception.get())
    {
        _proxy->__setRequestHandler(_delegate, new ConnectionRequestHandler(_reference, _connection, _compress));
    }
    _proxy = 0; // Break cyclic reference count.
    _delegate = 0; // Break cyclic reference count.
}


