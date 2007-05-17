// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/LocalException.h> // Need to be included before Outgoing.h because of std::auto_ptr<LocalException>
#include <IceE/Outgoing.h>
#include <IceE/ReplyStatus.h>
#include <IceE/Connection.h>
#include <IceE/Reference.h>
#include <IceE/Instance.h>
#include <IceE/Protocol.h>

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

IceInternal::Outgoing::Outgoing(Connection* connection, Reference* ref, const string& operation,
				OperationMode mode, const Context* context) :
    _connection(connection),
    _reference(ref),
    _state(StateUnsent),
    _stream(ref->getInstance().get(), ref->getInstance()->messageSizeMax()
#ifdef ICEE_HAS_WSTRING
            , ref->getInstance()->initializationData().stringConverter,
            ref->getInstance()->initializationData().wstringConverter
#endif
           )
{
    switch(_reference->getMode())
    {
	case ReferenceModeTwoway:
	case ReferenceModeOneway:
	{
	    _stream.writeBlob(requestHdr, sizeof(requestHdr));
	    break;
	}

	case ReferenceModeBatchOneway:
#ifdef ICEE_HAS_BATCH
	{
	    _connection->prepareBatchRequest(&_stream);
	    break;
	}
#endif
	case ReferenceModeDatagram:
	case ReferenceModeBatchDatagram:
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

    _stream.write(operation, false);

    _stream.write(static_cast<Byte>(mode));

    if(context == 0)
    {
        context = _reference->getContext();
    }

    _stream.writeSize(Int(context->size()));
    Context::const_iterator p;
    for(p = context->begin(); p != context->end(); ++p)
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
	case ReferenceModeTwoway:
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
		// Throw the exception wrapped in a LocalExceptionWrapper, to
		// indicate that the request cannot be resent without
		// potentially violating the "at-most-once" principle.
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
	
	case ReferenceModeOneway:
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

	case ReferenceModeBatchOneway:
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
	case ReferenceModeDatagram:
	case ReferenceModeBatchDatagram:
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
    if(_reference->getMode() == ReferenceModeBatchOneway)
    {
	_connection->abortBatchRequest();
	
	//
	// If we abort a batch requests, we cannot retry, because not
	// only the batch request that caused the problem will be
	// aborted, but all other requests in the batch as well.
	//
	throw LocalExceptionWrapper(ex, false);
    }
#endif
    
    ex.ice_throw();
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    assert(_reference->getMode() == ReferenceModeTwoway); // Can only be called for twoways.
    assert(_state <= StateInProgress);

    //
    // Only swap the stream if the given stream is not this Outgoing object stream!
    //
    if(&is != &_stream)
    {
	_stream.swap(is);
    }

    Byte replyStatus;
    _stream.read(replyStatus);
    
    switch(replyStatus)
    {
	case replyOK:
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
	
	case replyUserException:
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
	    _stream.read(operation, false);
	    
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
	    _stream.read(unknown, false);
	    
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
	    //_exception.reset(new UnknownReplyStatusException(__FILE__, __LINE__));
	    _exception.reset(new ProtocolException(__FILE__, __LINE__, "unknown reply status"));
	    _state = StateLocalException;
	    break;
	}
    }
}

void
IceInternal::Outgoing::finished(const LocalException& ex)
{
    assert(_reference->getMode() == ReferenceModeTwoway); // Can only be called for twoways.
    assert(_state <= StateInProgress);
    
    _state = StateLocalException;
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
}
