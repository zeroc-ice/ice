// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Outgoing.h>
#include <Ice/Object.h>
#include <Ice/ConnectionI.h>
#include <Ice/Reference.h>
#include <Ice/Endpoint.h>
#include <Ice/LocalException.h>
#include <Ice/Protocol.h>
#include <Ice/Instance.h>
#include <Ice/ReplyStatus.h>

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

IceInternal::Outgoing::Outgoing(ConnectionI* connection, Reference* ref, const string& operation,
                                OperationMode mode, const Context* context, bool compress) :
    _connection(connection),
    _reference(ref),
    _state(StateUnsent),
    _is(ref->getInstance().get()),
    _os(ref->getInstance().get()),
    _compress(compress)
{
    switch(_reference->getMode())
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
            _connection->prepareBatchRequest(&_os);
            break;
        }
    }

    try
    {
        _reference->getIdentity().__write(&_os);

        //
        // For compatibility with the old FacetPath.
        //
        if(_reference->getFacet().empty())
        {
            _os.write(static_cast<string*>(0), static_cast<string*>(0));
        }
        else
        {
            string facet = _reference->getFacet();
            _os.write(&facet, &facet + 1);
        }

        _os.write(operation, false);

        _os.write(static_cast<Byte>(mode));

        if(context != 0)
        {
            //
            // Explicit context
            //
            __writeContext(&_os, *context);
        }
        else
        {
            //
            // Implicit context
            //
            const ImplicitContextIPtr& implicitContext =
                _reference->getInstance()->getImplicitContext();
            
            const Context& prxContext = _reference->getContext()->getValue();

            if(implicitContext == 0)
            {
                __writeContext(&_os, prxContext);
            }
            else
            {
                implicitContext->write(prxContext, &_os);
            }
        }
        
        //
        // Input and output parameters are always sent in an
        // encapsulation, which makes it possible to forward requests as
        // blobs.
        //
        _os.startWriteEncaps();
    }
    catch(const LocalException& ex)
    {
        abort(ex);
    }
}

bool
IceInternal::Outgoing::invoke()
{
    assert(_state == StateUnsent);

    _os.endWriteEncaps();
    
    switch(_reference->getMode())
    {
        case Reference::ModeTwoway:
        {
            //
            // We let all exceptions raised by sending directly
            // propagate to the caller, because they can be retried
            // without violating "at-most-once". In case of such
            // exceptions, the connection object does not call back on
            // this object, so we don't need to lock the mutex, keep
            // track of state, or save exceptions.
            //
            _connection->sendRequest(&_os, this, _compress);
            
            //
            // Wait until the request has completed, or until the
            // request times out.
            //

            bool timedOut = false;

            {
                IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

                //
                // It's possible that the request has already
                // completed, due to a regular response, or because of
                // an exception. So we only change the state to "in
                // progress" if it is still "unsent".
                //
                if(_state == StateUnsent)
                {
                    _state = StateInProgress;
                }
                
                Int timeout = _connection->timeout();
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
                _connection->exception(TimeoutException(__FILE__, __LINE__));

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
                // well without violating "at-most-once".
                //
                if(dynamic_cast<CloseConnectionException*>(_exception.get()) ||
                   dynamic_cast<ObjectNotExistException*>(_exception.get()))
                {
                    _exception->ice_throw();
                }
                
                //
                // Throw the exception wrapped in a
                // LocalExceptionWrapper, to indicate that the request
                // cannot be resent without potentially violating the
                // "at-most-once" principle.
                //
                throw LocalExceptionWrapper(*_exception.get(), false);
            }
            
            if(_state == StateUserException)
            {
                return false;
            }

            assert(_state == StateOK);
            break;
        }
        
        case Reference::ModeOneway:
        case Reference::ModeDatagram:
        {
            //
            // For oneway and datagram requests, the connection object
            // never calls back on this object. Therefore we don't
            // need to lock the mutex or save exceptions. We simply
            // let all exceptions from sending propagate to the
            // caller, because such exceptions can be retried without
            // violating "at-most-once".
            //
            _state = StateInProgress;
            _connection->sendRequest(&_os, 0, _compress);
            break;
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
            _connection->finishBatchRequest(&_os, _compress);
            break;
        }
    }

    return true;
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
    if(_reference->getMode() == Reference::ModeBatchOneway || _reference->getMode() == Reference::ModeBatchDatagram)
    {
        _connection->abortBatchRequest();
        
        //
        // If we abort a batch requests, we cannot retry, because not
        // only the batch request that caused the problem will be
        // aborted, but all other requests in the batch as well.
        //
        throw LocalExceptionWrapper(ex, false);
    }
    
    ex.ice_throw();
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);

    assert(_reference->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);

    _is.swap(is);
    Byte replyStatus;
    _is.read(replyStatus);
    
    switch(replyStatus)
    {
        case replyOK:
        {
            //
            // Input and output parameters are always sent in an
            // encapsulation, which makes it possible to forward
            // oneway requests as blobs.
            //
            _is.startReadEncaps();
            _state = StateOK; // The state must be set last, in case there is an exception.
            break;
        }
        
        case replyUserException:
        {
            //
            // Input and output parameters are always sent in an
            // encapsulation, which makes it possible to forward
            // oneway requests as blobs.
            //
            _is.startReadEncaps();
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
IceInternal::Outgoing::finished(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(_monitor);
    
    assert(_reference->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);
    
    _state = StateLocalException;
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
    _monitor.notify();
}
