// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LocalException.h> // Need to be included before Outgoing.h because of std::auto_ptr<LocalException>
#include <IceE/Outgoing.h>
#include <IceE/DispatchStatus.h>
#include <IceE/Connection.h>
#include <IceE/Reference.h>
#include <IceE/Instance.h>
#include <IceE/Protocol.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::NonRepeatable::NonRepeatable(const NonRepeatable& ex)
{
    _ex.reset(dynamic_cast<LocalException*>(ex.get()->ice_clone()));
}

IceInternal::NonRepeatable::NonRepeatable(const ::Ice::LocalException& ex)
{
    _ex.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
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
    _stream(ref->getInstance().get(), ref->getInstance()->messageSizeMax())
{
    switch(_reference->getMode())
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	{
	    _stream.writeBlob(&(_connection->getRequestHeader()[0]), headerSize + sizeof(Int));
	    break;
	}

	case Reference::ModeBatchOneway:
#ifdef ICEE_HAS_BATCH
	{
	    _connection->prepareBatchRequest(&_stream);
	    break;
	}
#endif
	case Reference::ModeDatagram:
	case Reference::ModeBatchDatagram:
	{
	    assert(false);
	    break;
	}
    }

//    _reference->getIdentity().__write(&_stream);
    _stream.write(_reference->getIdentity().name); // Directly write name for performance reasons.
    _stream.write(_reference->getIdentity().category); // Directly write category for performance reasons.

    //
    // For compatibility with the old FacetPath we still write an
    // array of strings (we don't use the basic stream string array
    // method here for performance reasons.)
    //
    if(_reference->getFacet().empty())
    {
	_stream.writeSize(0);
    }
    else
    {
	_stream.writeSize(1);
	_stream.write(_reference->getFacet());
    }

    _stream.write(operation);

    _stream.write(static_cast<Byte>(mode));

    _stream.writeSize(Int(context.size()));
    Context::const_iterator p;
    for(p = context.begin(); p != context.end(); ++p)
    {
	_stream.write(p->first);
	_stream.write(p->second);
    }
    
    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward requests as
    // blobs.
    //
    _stream.startWriteEncaps();
}

bool
IceInternal::Outgoing::invoke()
{
    assert(_state == StateUnsent);
    _state = StateInProgress;

    _stream.endWriteEncaps();
    
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
	    _connection->sendRequest(&_stream, this);

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
	{
	    //
	    // For oneway requests, the connection object
	    // never calls back on this object. Therefore we don't
	    // need to lock the mutex or save exceptions. We simply
	    // let all exceptions from sending propagate to the
	    // caller, because such exceptions can be retried without
	    // violating "at-most-once".
	    //
	    _connection->sendRequest(&_stream, 0);
	    break;
	}

	case Reference::ModeBatchOneway:
#ifdef ICEE_HAS_BATCH
	{
	    //
	    // For batch oneways, the same rules as for
	    // regular oneways (see comment above)
	    // apply.
	    //
	    _connection->finishBatchRequest(&_stream);
	    break;
	}
#endif
	case Reference::ModeDatagram:
	case Reference::ModeBatchDatagram:
	{
	    assert(false);
	    return false;
	}
    }

    return true;
}

void
IceInternal::Outgoing::abort(const LocalException& ex)
{
    assert(_state == StateUnsent);
    
    //
    // If we didn't finish a batch oneway request, we must
    // notify the connection about that we give up ownership of the
    // batch stream.
    //
#ifdef ICEE_HAS_BATCH
    if(_reference->getMode() == Reference::ModeBatchOneway)
    {
	_connection->abortBatchRequest();
	
	//
	// If we abort a batch requests, we cannot retry, because not
	// only the batch request that caused the problem will be
	// aborted, but all other requests in the batch as well.
	//
	throw NonRepeatable(ex);
    }
#endif
    
    ex.ice_throw();
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    assert(_reference->getMode() == Reference::ModeTwoway); // Can only be called for twoways.
    assert(_state <= StateInProgress);

    //
    // Only swap the stream if the given stream is not this Outgoing object stream!
    //
    if(&is != &_stream)
    {
	_stream.swap(is);
    }

    Byte status;
    _stream.read(status);
    
    switch(static_cast<DispatchStatus>(status))
    {
	case DispatchOK:
	{
	    //
	    // Input and output parameters are always sent in an
	    // encapsulation, which makes it possible to forward
	    // oneway requests as blobs.
	    //
	    _stream.startReadEncaps();
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
	    _stream.startReadEncaps();
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
	    ident.__read(&_stream);

	    //
	    // For compatibility with the old FacetPath.
	    //
	    vector<string> facetPath;
	    _stream.read(facetPath);
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
	    _stream.read(operation);
	    
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
	    _exception.reset(ex);

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
	    _stream.read(unknown);
	    
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
}

void
IceInternal::Outgoing::finished(const LocalException& ex)
{
    assert(_reference->getMode() == Reference::ModeTwoway); // Can only be called for twoways.
    assert(_state <= StateInProgress);
    
    _state = StateLocalException;
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
}
