// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/Outgoing.h>
#include <Ice/Object.h>
#include <Ice/Connection.h>
#include <Ice/Reference.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::NonRepeatable::NonRepeatable(const NonRepeatable& ex)
{
    _ex = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.get()->ice_clone()));
}

IceInternal::NonRepeatable::NonRepeatable(const ::Ice::LocalException& ex)
{
    _ex = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
}

const ::Ice::LocalException*
IceInternal::NonRepeatable::get() const
{
    assert(_ex.get());
    return _ex.get();
}

IceInternal::Outgoing::Outgoing(Connection* connection, Reference* ref, const string& operation,
				OperationMode mode, const Context& context) :
    _connection(connection),
    _reference(ref),
    _state(StateUnsent),
    _is(ref->instance.get()),
    _os(ref->instance.get())
{
    switch(_reference->mode)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeDatagram:
	{
	    _connection->prepareRequest(&_os);
	    break;
	}

	case Reference::ModeBatchOneway:
	case Reference::ModeBatchDatagram:
	{
	    _connection->prepareBatchRequest(&_os);
	    break;
	}
    }

    _reference->identity.__write(&_os);

    //
    // For compatibility with the old FacetPath.
    //
    if(_reference->facet.empty())
    {
	_os.write(vector<string>());
    }
    else
    {
	vector<string> facetPath;
	facetPath.push_back(_reference->facet);
	_os.write(facetPath);
    }

    _os.write(operation);

    _os.write(static_cast<Byte>(mode));

    _os.writeSize(Int(context.size()));
    Context::const_iterator p;
    for(p = context.begin(); p != context.end(); ++p)
    {
	_os.write(p->first);
	_os.write(p->second);
    }
    
    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward requests as
    // blobs.
    //
    _os.startWriteEncaps();
}

bool
IceInternal::Outgoing::invoke()
{
    _os.endWriteEncaps();
    
    switch(_reference->mode)
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
	    _connection->sendRequest(&_os, this);
	    
	    //
	    // Wait until the request has completed, or until the
	    // request times out.
	    //

	    bool timedOut = false;

	    {
		IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

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
			timedWait(IceUtil::Time::milliSeconds(timeout));
			
			if(_state == StateInProgress)
			{
			    timedOut = true;
			}
		    }
		    else
		    {
			wait();
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
		    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
		    
		    while(_state == StateInProgress)
		    {
			wait();
		    }

		    assert(_exception.get());
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
		if(dynamic_cast<const CloseConnectionException*>(_exception.get()))
		{
		    _exception->ice_throw();
		}
		
		//
		// Throw the exception wrapped in a NonRepeatable, to
		// indicate that the request cannot be resent without
		// potentially violating the "at-most-once" principle.
		//
		throw NonRepeatable(*_exception.get());
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
	    // need to lock the mutex, keep track of state, or save
	    // exceptions. We simply let all exceptions from sending
	    // propagate to the caller, because such exceptions can be
	    // retried without violating "at-most-once".
	    //
	    _connection->sendRequest(&_os, 0);
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
	    _connection->finishBatchRequest(&_os);
	    break;
	}
    }

    return true;
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    assert(_reference->mode == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);

    _is.swap(is);
    Byte status;
    _is.read(status);
    
    switch(static_cast<DispatchStatus>(status))
    {
	case DispatchOK:
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
	
	case DispatchUserException:
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
	
	case DispatchObjectNotExist:
	case DispatchFacetNotExist:
	case DispatchOperationNotExist:
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
	    if(!facetPath.empty()) // TODO: Throw an exception if facetPath has more than one element?
	    {
		facet.swap(facetPath[0]);
	    }

	    string operation;
	    _is.read(operation);
	    
	    RequestFailedException* ex;
	    switch(static_cast<DispatchStatus>(status))
	    {
		case DispatchObjectNotExist:
		{
		    ex = new ObjectNotExistException(__FILE__, __LINE__);
		    break;
		}
		
		case DispatchFacetNotExist:
		{
		    ex = new FacetNotExistException(__FILE__, __LINE__);
		    break;
		}
		
		case DispatchOperationNotExist:
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
	    _exception = auto_ptr<LocalException>(ex);

	    _state = StateLocalException; // The state must be set last, in case there is an exception.
	    break;
	}
	
	case DispatchUnknownException:
	case DispatchUnknownLocalException:
	case DispatchUnknownUserException:
	{
	    //
	    // Don't read the exception members directly into the
	    // exception. Otherwise if reading fails and raises an
	    // exception, you will have a memory leak.
	    //
	    string unknown;
	    _is.read(unknown);
	    
	    UnknownException* ex;
	    switch(static_cast<DispatchStatus>(status))
	    {
		case DispatchUnknownException:
		{
		    ex = new UnknownException(__FILE__, __LINE__);
		    break;
		}
		
		case DispatchUnknownLocalException:
		{
		    ex = new UnknownLocalException(__FILE__, __LINE__);
		    break;
		}
		
		case DispatchUnknownUserException:
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
	    _exception = auto_ptr<LocalException>(ex);

	    _state = StateLocalException; // The state must be set last, in case there is an exception.
	    break;
	}
	
	default:
	{
	    _exception = auto_ptr<LocalException>(new UnknownReplyStatusException(__FILE__, __LINE__));
	    _state = StateLocalException;
	    break;
	}
    }

    notify();
}

void
IceInternal::Outgoing::finished(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    assert(_reference->mode == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);
    
    _state = StateLocalException;
    _exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
    notify();
}
