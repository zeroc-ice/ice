// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Outgoing.h>
#include <Ice/Object.h>
#include <Ice/RequestHandler.h>
#include <Ice/ConnectionI.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/Instance.h>
#include <Ice/ReplyStatus.h>
#include <Ice/Observer.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::LocalExceptionWrapper::LocalExceptionWrapper(const LocalException& ex, bool r) :
    _retry(r)
{
    _ex.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
}

IceInternal::LocalExceptionWrapper::LocalExceptionWrapper(const LocalExceptionWrapper& ex) :
    _retry(ex._retry)
{
    _ex.reset(dynamic_cast<LocalException*>(ex.get()->ice_clone()));
}

void
IceInternal::LocalExceptionWrapper::throwWrapper(const std::exception& ex)
{

    const UserException* ue = dynamic_cast<const UserException*>(&ex);
    if(ue)
    {
        stringstream s;
        s << *ue;
        throw LocalExceptionWrapper(UnknownUserException(__FILE__, __LINE__, s.str()), false);
    }

    const LocalException* le = dynamic_cast<const LocalException*>(&ex);
    if(le)
    {
        if(dynamic_cast<const UnknownException*>(le) ||
           dynamic_cast<const ObjectNotExistException*>(le) ||
           dynamic_cast<const OperationNotExistException*>(le) ||
           dynamic_cast<const FacetNotExistException*>(le))
        {
            throw LocalExceptionWrapper(*le, false);
        }
        stringstream s;
        s << *le;
#ifdef __GNUC__
        s << "\n" << le->ice_stackTrace();
#endif
        throw LocalExceptionWrapper(UnknownLocalException(__FILE__, __LINE__, s.str()), false);
    }
    string msg = "std::exception: ";
    throw LocalExceptionWrapper(UnknownException(__FILE__, __LINE__, msg + ex.what()), false);
}

const LocalException*
IceInternal::LocalExceptionWrapper::get() const
{
    assert(_ex.get());
    return _ex.get();
}

bool
IceInternal::LocalExceptionWrapper::retry() const
{
    return _retry;
}

IceInternal::Outgoing::Outgoing(RequestHandler* handler, const string& operation, OperationMode mode, 
                                const Context* context) :
    _handler(handler),
    _state(StateUnsent),
    _encoding(handler->getReference()->getEncoding()),
    _is(handler->getReference()->getInstance().get(), Ice::currentProtocolEncoding),
    _os(handler->getReference()->getInstance().get(), Ice::currentProtocolEncoding),
    _sent(false)
{ 
    const ObserverResolverPtr& resolver = _handler->getReference()->getInstance()->initializationData().observerResolver;
    if(resolver)
    {
        try
        {
            _observer = resolver->getInvocationObserver(handler->getProxy(), 
                                                        operation, 
                                                        *context, 
                                                        handler->getConnection(false));
        }
        catch(const Ice::LocalException&)
        {
            //
            // Ignore: can be raised by getConnection is no connection could be obtained,
            // the request will be re-tried in this case.
            //
        }

        if(_observer)
        {
            _observer->attach();
        }
    }

    switch(_handler->getReference()->getMode())
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
            _handler->prepareBatchRequest(&_os);
            break;
        }
    }

    try
    {
        _handler->getReference()->getIdentity().__write(&_os);

        //
        // For compatibility with the old FacetPath.
        //
        if(_handler->getReference()->getFacet().empty())
        {
            _os.write(static_cast<string*>(0), static_cast<string*>(0));
        }
        else
        {
            string facet = _handler->getReference()->getFacet();
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
            const ImplicitContextIPtr& implicitContext = _handler->getReference()->getInstance()->getImplicitContext();
            const Context& prxContext = _handler->getReference()->getContext()->getValue();
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
    if(_observer)
    {
        _observer->detach();
    }
}

bool
IceInternal::Outgoing::invoke()
{
    assert(_state == StateUnsent);

    switch(_handler->getReference()->getMode())
    {
        case Reference::ModeTwoway:
        {
            _state = StateInProgress;

            Ice::ConnectionI* connection = _handler->sendRequest(this);
            assert(connection);

            bool timedOut = false;
    
            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

                //
                // If the request is being sent in the background we first wait for the
                // sent notification.
                //
                while(_state != StateFailed && !_sent)
                {
                    _monitor.wait();
                }

                //
                // Wait until the request has completed, or until the request times out.
                //
    
                Int timeout = connection->timeout();
                while(_state == StateInProgress && !timedOut)
                {
                    if(timeout >= 0)
                    {   
                        _monitor.timedWait(IceUtil::Time::milliSeconds(timeout));
                        
                        if(_state == StateInProgress)
                        {
                            timedOut = true;
                        }
                    }
                    else
                    {
                        _monitor.wait();
                    }
                }
            }

            if(timedOut)
            {
                //
                // Must be called outside the synchronization of this
                // object.
                //
                connection->exception(TimeoutException(__FILE__, __LINE__));

                //
                // We must wait until the exception set above has
                // propagated to this Outgoing object.
                //
                {
                    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                    while(_state == StateInProgress)
                    {
                        _monitor.wait();
                    }
                }
            }

            if(_exception.get())
            {
                //
                // A CloseConnectionException indicates graceful
                // server shutdown, and is therefore always repeatable
                // without violating "at-most-once". That's because by
                // sending a close connection message, the server
                // guarantees that all outstanding requests can safely
                // be repeated.
                //
                // An ObjectNotExistException can always be retried as
                // well without violating "at-most-once" (see the
                // implementation of the checkRetryAfterException
                // method of the ProxyFactory class for the reasons
                // why it can be useful).
                //
                if(!_sent ||
                   dynamic_cast<CloseConnectionException*>(_exception.get()) ||
                   dynamic_cast<ObjectNotExistException*>(_exception.get()))
                {
                    _exception->ice_throw();
                }
                
                //
                // Throw the exception wrapped in a LocalExceptionWrapper, 
                // to indicate that the request cannot be resent without 
                // potentially violating the "at-most-once" principle.
                //
                throw LocalExceptionWrapper(*_exception.get(), false);
            }
            
            if(_state == StateUserException)
            {
                return false;
            }
            else
            {
                assert(_state == StateOK);
                return true;
            }
        }

        case Reference::ModeOneway:
        case Reference::ModeDatagram:
        {
            _state = StateInProgress;
            if(_handler->sendRequest(this))
            {
                //
                // If the handler returns the connection, we must wait for the sent callback.
                //
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
                while(_state != StateFailed && !_sent)
                {
                    _monitor.wait();
                }

                if(_exception.get())
                {
                    _exception->ice_throw();
                }
            }
            return true;
        }

        case Reference::ModeBatchOneway:
        case Reference::ModeBatchDatagram:
        {
            //
            // For batch oneways and datagrams, the same rules as for
            // regular oneways and datagrams (see comment above)
            // apply.
            //
            _state = StateInProgress;
            _handler->finishBatchRequest(&_os);
            return true;
        }
    }

    assert(false);
    return false;
}

void
IceInternal::Outgoing::abort(const LocalException& ex)
{
    assert(_state == StateUnsent);
    
    //
    // If we didn't finish a batch oneway or datagram request, we must
    // notify the connection about that we give up ownership of the
    // batch stream.
    //
    if(_handler->getReference()->getMode() == Reference::ModeBatchOneway || 
       _handler->getReference()->getMode() == Reference::ModeBatchDatagram)
    {
        _handler->abortBatchRequest();
    }
    
    ex.ice_throw();
}

void
IceInternal::Outgoing::sent(bool notify)
{
    if(notify)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _sent = true;
        _monitor.notify();
    }
    else
    {
        //
        // No synchronization is necessary if called from sendRequest() because the connection
        // send mutex is locked and no other threads can call on Outgoing until it's released.
        //
        _sent = true;
    }
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    assert(_handler->getReference()->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);

    _is.swap(is);
    Byte replyStatus;
    _is.read(replyStatus);
    
    switch(replyStatus)
    {
        case replyOK:
        {
            if(_observer)
            {
                _observer->responseOK();
            }
            _state = StateOK; // The state must be set last, in case there is an exception.
            break;
        }
        
        case replyUserException:
        {
            if(_observer)
            {
                _observer->responseUserException();
            }
            _state = StateUserException; // The state must be set last, in case there is an exception.
            break;
        }
        
        case replyObjectNotExist:
        case replyFacetNotExist:
        case replyOperationNotExist:
        {
            if(_observer)
            {
                _observer->responseRequestFailedException();
            }

            //
            // Don't read the exception members directly into the
            // exception. Otherwise if reading fails and raises an
            // exception, you will have a memory leak.
            //
            Identity ident;
            ident.__read(&_is);

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
            if(_observer)
            {
                _observer->responseUnknownException();
            }

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
            if(_observer)
            {
                _observer->responseUnknownException();
            }
            _exception.reset(new UnknownReplyStatusException(__FILE__, __LINE__));
            _state = StateLocalException;
            break;
        }
    }

    _monitor.notify();
}

void
IceInternal::Outgoing::finished(const LocalException& ex, bool sent)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    assert(_state <= StateInProgress);
    _state = StateFailed;
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
    _sent = sent;
    _monitor.notify();
}

void
IceInternal::Outgoing::throwUserException()
{
    try
    {
        if(_observer)
        {
            _watch.start();
        }
        _is.startReadEncaps();
        _is.throwException();
    }
    catch(const Ice::UserException&)
    {
        _is.endReadEncaps();
        if(_observer)
        {
            _observer->unmarshalTime(_watch.stop());
        }
        throw;
    }
}

IceInternal::BatchOutgoing::BatchOutgoing(RequestHandler* handler) :
    _handler(handler),
    _connection(0),
    _sent(false),
    _os(handler->getReference()->getInstance().get(), Ice::currentProtocolEncoding)
{
}

IceInternal::BatchOutgoing::BatchOutgoing(ConnectionI* connection, Instance* instance) :
    _handler(0),
    _connection(connection),
    _sent(false), 
    _os(instance, Ice::currentProtocolEncoding)
{
}

void
IceInternal::BatchOutgoing::invoke()
{
    assert(_handler || _connection);
    if((_handler && !_handler->flushBatchRequests(this)) || (_connection && !_connection->flushBatchRequests(this)))
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        while(!_exception.get() && !_sent)
        {
            _monitor.wait();
        }
        
        if(_exception.get())
        {
            _exception->ice_throw();
        }
    }
}

void
IceInternal::BatchOutgoing::sent(bool notify)
{
    if(notify)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
        _sent = true;
        _monitor.notify();
    }
    else
    {
        _sent = true;
    }
}

void
IceInternal::BatchOutgoing::finished(const Ice::LocalException& ex, bool)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
    _monitor.notify();
}
