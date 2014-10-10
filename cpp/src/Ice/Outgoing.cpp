// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/Outgoing.h>
#include <Ice/ConnectionI.h>
#include <Ice/CollocatedRequestHandler.h>
#include <Ice/Reference.h>
#include <Ice/Instance.h>
#include <Ice/LocalException.h>
#include <Ice/ReplyStatus.h>
#include <Ice/ImplicitContextI.h>

using namespace std;
using namespace Ice;
using namespace Ice::Instrumentation;
using namespace IceInternal;

OutgoingBase::OutgoingBase(Instance* instance, const string& operation) :
    _os(instance, Ice::currentProtocolEncoding), _sent(false)
{
}

Outgoing::Outgoing(IceProxy::Ice::Object* proxy, const string& operation, OperationMode mode, const Context* context) :
    OutgoingBase(proxy->__reference()->getInstance().get(), operation),
    _proxy(proxy),
    _mode(mode),
    _state(StateUnsent),
    _encoding(getCompatibleEncoding(proxy->__reference()->getEncoding())),
    _is(proxy->__reference()->getInstance().get(), Ice::currentProtocolEncoding)
{ 
    checkSupportedProtocol(getCompatibleProtocol(proxy->__reference()->getProtocol()));

    _observer.attach(proxy, operation, context);

    int invocationTimeout = _proxy->__reference()->getInvocationTimeout();
    if(invocationTimeout > 0)
    {
        _invocationTimeoutDeadline = IceUtil::Time::now() + IceUtil::Time::milliSeconds(invocationTimeout);
    }

    switch(_proxy->__reference()->getMode())
    {
        case Reference::ModeTwoway:
        case Reference::ModeOneway:
        case Reference::ModeDatagram:
        {
            _os.writeBlob(requestHdr, sizeof(requestHdr));
            break;
        }

        case Reference::ModeBatchOneway:
        case Reference::ModeBatchDatagram:
        {
            while(true)
            {
                try
                {
                    _handler = proxy->__getRequestHandler();
                    _handler->prepareBatchRequest(&_os);
                    break;
                }
                catch(const RetryException&)
                {
                    _proxy->__setRequestHandler(_handler, 0); // Clear request handler and retry.
                }
                catch(const Ice::LocalException& ex)
                {
                    _observer.failed(ex.ice_name());
                    _proxy->__setRequestHandler(_handler, 0); // Clear request handler
                    throw;
                }
            }
            break;
        }
    }

    try
    {
        _os.write(_proxy->__reference()->getIdentity());

        //
        // For compatibility with the old FacetPath.
        //
        if(_proxy->__reference()->getFacet().empty())
        {
            _os.write(static_cast<string*>(0), static_cast<string*>(0));
        }
        else
        {
            string facet = _proxy->__reference()->getFacet();
            _os.write(&facet, &facet + 1);
        }

        _os.write(operation, false);

        _os.write(static_cast<Byte>(mode));

        if(context != 0)
        {
            //
            // Explicit context
            //
            _os.write(*context);
        }
        else
        {
            //
            // Implicit context
            //
            const ImplicitContextIPtr& implicitContext = _proxy->__reference()->getInstance()->getImplicitContext();
            const Context& prxContext = _proxy->__reference()->getContext()->getValue();
            if(implicitContext == 0)
            {
                _os.write(prxContext);
            }
            else
            {
                implicitContext->write(prxContext, &_os);
            }
        }
    }
    catch(const LocalException& ex)
    {
        abort(ex);
    }
}

Outgoing::~Outgoing()
{
}

bool
Outgoing::send(const Ice::ConnectionIPtr& connection, bool compress, bool response)
{
    return connection->sendRequest(this, compress, response);
}

void
Outgoing::invokeCollocated(CollocatedRequestHandler* handler)
{
    handler->invokeRequest(this);
}

void
Outgoing::sent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    if(_proxy->__reference()->getMode() != Reference::ModeTwoway)
    {
        _childObserver.detach();
        _state = StateOK;
    }
    _sent = true;
    _monitor.notify();

    //
    // NOTE: At this point the stack allocated Outgoing object can be destroyed 
    // since the notify() on the monitor will release the thread waiting on the
    // synchronous Ice call.
    //
}

void
Outgoing::completed(const Exception& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    //assert(_state <= StateInProgress);
    if(_state > StateInProgress)
    {
        //
        // Response was already received but message
        // didn't get removed first from the connection
        // send message queue so it's possible we can be
        // notified of failures. In this case, ignore the
        // failure and assume the outgoing has been sent.
        //
        assert(_state != StateFailed);
        _sent = true;
        _monitor.notify();
        return;
    }

    _childObserver.failed(ex.ice_name());
    _childObserver.detach();

    _state = StateFailed;
    _exception.reset(ex.ice_clone());
    _monitor.notify();
}

bool
Outgoing::invoke()
{
    assert(_state == StateUnsent);
    
    const Reference::Mode mode = _proxy->__reference()->getMode();
    if(mode == Reference::ModeBatchOneway || mode == Reference::ModeBatchDatagram)
    {
        _state = StateInProgress;
        _handler->finishBatchRequest(&_os);
        return true;
    }

    int cnt = 0;
    while(true)
    {        
        try
        {
            if(_invocationTimeoutDeadline != IceUtil::Time() && _invocationTimeoutDeadline <= IceUtil::Time::now())
            {
                throw Ice::InvocationTimeoutException(__FILE__, __LINE__);
            }

            _state = StateInProgress;
            _exception.reset(0);
            _sent = false;

            _handler = _proxy->__getRequestHandler();

            if(_handler->sendRequest(this)) // Request sent and no response expected, we're done.
            {
                return true;
            }
                    
            bool timedOut = false;
            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                        
                //
                // If the handler says it's not finished, we wait until we're done.
                //
                if(_invocationTimeoutDeadline != IceUtil::Time())
                {
                    IceUtil::Time now = IceUtil::Time::now();
                    timedOut = now >= _invocationTimeoutDeadline;
                    while((_state == StateInProgress || !_sent) && _state != StateFailed && !timedOut)
                    {
                        _monitor.timedWait(_invocationTimeoutDeadline - now);
                            
                        if((_state == StateInProgress || !_sent) && _state != StateFailed)
                        {
                            now = IceUtil::Time::now();
                            timedOut = now >= _invocationTimeoutDeadline;
                        }
                    }
                }
                else
                {
                    while((_state == StateInProgress || !_sent) && _state != StateFailed)
                    {
                        _monitor.wait();
                    }
                }
            }
                
            if(timedOut)
            {
                _handler->requestCanceled(this, InvocationTimeoutException(__FILE__, __LINE__));

                //
                // Wait for the exception to propagate. It's possible the request handler ignores
                // the timeout if there was a failure shortly before requestCanceled got called. 
                // In this case, the exception should be set on the Outgoing.
                //
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                while(_state == StateInProgress)
                {
                    _monitor.wait();
                }
            }
                
            if(_exception.get())
            {
                _exception->ice_throw();
            }
            else
            {
                assert(_state != StateInProgress);
                return _state == StateOK;
            }
        }
        catch(const RetryException&)
        {
            _proxy->__setRequestHandler(_handler, 0); // Clear request handler and retry.
        }
        catch(const Ice::Exception& ex)
        {
            try
            {
                IceUtil::Time interval;
                interval = IceUtil::Time::milliSeconds(_proxy->__handleException(ex, _handler, _mode, _sent, cnt));
                if(interval > IceUtil::Time())
                {
                    if(_invocationTimeoutDeadline != IceUtil::Time())
                    {
                        IceUtil::Time deadline = _invocationTimeoutDeadline - IceUtil::Time::now();
                        if(deadline < interval)
                        {
                            interval = deadline; 
                        }
                    }
                    IceUtil::ThreadControl::sleep(interval);
                }
                if(_invocationTimeoutDeadline == IceUtil::Time() || _invocationTimeoutDeadline > IceUtil::Time::now())
                {
                    _observer.retried();
                }
            }
            catch(const Ice::Exception& ex)
            {
                _observer.failed(ex.ice_name());
                throw;
            }
        }
    }

    assert(false);
    return false;
}

void
Outgoing::abort(const LocalException& ex)
{
    assert(_state == StateUnsent);
    
    //
    // If we didn't finish a batch oneway or datagram request, we must
    // notify the connection about that we give up ownership of the
    // batch stream.
    //
    if(_proxy->__reference()->getMode() == Reference::ModeBatchOneway || 
       _proxy->__reference()->getMode() == Reference::ModeBatchDatagram)
    {
        _handler->abortBatchRequest();
    }
    
    ex.ice_throw();
}

void
Outgoing::completed(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    assert(_proxy->__reference()->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);
    if(_childObserver)
    {
        _childObserver->reply(static_cast<Int>(is.b.size() - headerSize - 4));
    }
    _childObserver.detach();

    _is.swap(is);

    Byte replyStatus;
    _is.read(replyStatus);
    
    switch(replyStatus)
    {
        case replyOK:
        {
            _state = StateOK; // The state must be set last, in case there is an exception.
            break;
        }
        
        case replyUserException:
        {
            _observer.userException();
            _state = StateUserException; // The state must be set last, in case there is an exception.
            break;
        }
        
        case replyObjectNotExist:
        case replyFacetNotExist:
        case replyOperationNotExist:
        {
            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            Identity ident;
            _is.read(ident);

            //
            // For compatibility with the old FacetPath.
            //
            vector<string> facetPath;
            _is.read(facetPath);
            string facet;
            if(!facetPath.empty())
            {
                if(facetPath.size() > 1)
                {
                    throw MarshalException(__FILE__, __LINE__);
                }
                facet.swap(facetPath[0]);
            }

            string operation;
            _is.read(operation, false);
            
            RequestFailedException* ex;
            switch(replyStatus)
            {
                case replyObjectNotExist:
                {
                    ex = new ObjectNotExistException(__FILE__, __LINE__);
                    break;
                }
                
                case replyFacetNotExist:
                {
                    ex = new FacetNotExistException(__FILE__, __LINE__);
                    break;
                }
                
                case replyOperationNotExist:
                {
                    ex = new OperationNotExistException(__FILE__, __LINE__);
                    break;
                }
                
                default:
                {
                    ex = 0; // To keep the compiler from complaining.
                    assert(false);
                    break;
                }
            }
            
            ex->id = ident;
            ex->facet = facet;
            ex->operation = operation;
            _exception.reset(ex);

            _state = StateLocalException; // The state must be set last, in case there is an exception.
            break;
        }
        
        case replyUnknownException:
        case replyUnknownLocalException:
        case replyUnknownUserException:
        {
            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            string unknown;
            _is.read(unknown, false);
            
            UnknownException* ex;
            switch(replyStatus)
            {
                case replyUnknownException:
                {
                    ex = new UnknownException(__FILE__, __LINE__);
                    break;
                }
                
                case replyUnknownLocalException:
                {
                    ex = new UnknownLocalException(__FILE__, __LINE__);
                    break;
                }
                
                case replyUnknownUserException:
                {
                    ex = new UnknownUserException(__FILE__, __LINE__);
                    break;
                }
                
                default:
                {
                    ex = 0; // To keep the compiler from complaining.
                    assert(false);
                    break;
                }
            }
            
            ex->unknown = unknown;
            _exception.reset(ex);

            _state = StateLocalException; // The state must be set last, in case there is an exception.
            break;
        }
        
        default:
        {
            _exception.reset(new UnknownReplyStatusException(__FILE__, __LINE__));
            _state = StateLocalException;
            break;
        }
    }

    _monitor.notify();
}

void
Outgoing::throwUserException()
{
    try
    {
        _is.startReadEncaps();
        _is.throwException();
    }
    catch(const Ice::UserException&)
    {
        _is.endReadEncaps();
        throw;
    }
}

FlushBatch::FlushBatch(IceProxy::Ice::Object* proxy, const string& operation) :
    OutgoingBase(proxy->__reference()->getInstance().get(), operation), _proxy(proxy), _connection(0)
{
    checkSupportedProtocol(proxy->__reference()->getProtocol());

    _observer.attach(proxy->__reference()->getInstance().get(), operation);
}

FlushBatch::FlushBatch(ConnectionI* connection, Instance* instance, const string& operation) :
    OutgoingBase(instance, operation), _proxy(0), _connection(connection)
{
    _observer.attach(instance, operation);
}

void
FlushBatch::invoke()
{
    assert(_proxy || _connection);

    if(_connection)
    {
        if(_connection->flushBatchRequests(this))
        {
            return;
        }

        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        while(!_exception.get() && !_sent)
        {
            _monitor.wait();
        }
        if(_exception.get())
        {
            _exception->ice_throw();
        }
        return;
    }

    RequestHandlerPtr handler;
    try
    {
        handler = _proxy->__getRequestHandler();
        if(handler->sendRequest(this))
        {
            return;
        }

        bool timedOut = false;
        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
            int timeout = _proxy->__reference()->getInvocationTimeout();
            if(timeout > 0)
            {
                IceUtil::Time now = IceUtil::Time::now();
                IceUtil::Time deadline = now + IceUtil::Time::milliSeconds(timeout);
                while(!_exception.get() && !_sent && !timedOut)
                {
                    _monitor.timedWait(deadline - now);                
                    if(!_exception.get() && !_sent)
                    {
                        now = IceUtil::Time::now();
                        timedOut = now >= deadline;
                    }
                }
            }
            else
            {
                while(!_exception.get() && !_sent)
                {
                    _monitor.wait();
                }
            }
        }

        if(timedOut)
        {
            Ice::InvocationTimeoutException ex(__FILE__, __LINE__);
            handler->requestCanceled(this, ex);

            //
            // Wait for the exception to propagate. It's possible the request handler ignores
            // the timeout if there was a failure shortly before requestTimedOut got called. 
            // In this case, the exception should be set on the Outgoing.
            //
            IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
            while(!_exception.get())
            {
                _monitor.wait();
            }
        }
    
        if(_exception.get())
        {
            _exception->ice_throw();
        }
    }
    catch(const RetryException&)
    {
        //
        // Clear request handler but don't retry or throw. Retrying
        // isn't useful, there were no batch requests associated with
        // the proxy's request handler.
        //
        _proxy->__setRequestHandler(handler, 0); 
    }
    catch(const Ice::Exception& ex)
    {
        _proxy->__setRequestHandler(handler, 0); // Clear request handler
        _observer.failed(ex.ice_name());
        throw; // Throw to notify the user that batch requests were potentially lost.
    }
}

bool
FlushBatch::send(const Ice::ConnectionIPtr& connection, bool, bool)
{
    return connection->flushBatchRequests(this);
}

void
FlushBatch::invokeCollocated(CollocatedRequestHandler* handler)
{
    handler->invokeBatchRequests(this);
}

void
FlushBatch::sent()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    _childObserver.detach();
    
    _sent = true;
    _monitor.notify();

    //
    // NOTE: At this point the stack allocated FlushBatch object
    // can be destroyed since the notify() on the monitor will release
    // the thread waiting on the synchronous Ice call.
    //
}

void
FlushBatch::completed(const Ice::Exception& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    _childObserver.failed(ex.ice_name());
    _childObserver.detach();
    _exception.reset(ex.ice_clone());
    _monitor.notify();
}
