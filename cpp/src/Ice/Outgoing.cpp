// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Outgoing.h>
#include <Ice/Object.h>
#include <Ice/Emitter.h>
#include <Ice/Reference.h>
#include <Ice/Exception.h>

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

IceInternal::Outgoing::Outgoing(const EmitterPtr& emitter, const ReferencePtr& ref, const char* operation,
				const Context& context) :
    _emitter(emitter),
    _reference(ref),
    _state(StateUnsent),
    _is(ref->instance),
    _os(ref->instance)
{
    switch (_reference->mode)
    {
	case Reference::ModeTwoway:
	case Reference::ModeOneway:
	case Reference::ModeDatagram:
	{
	    _emitter->prepareRequest(this);
	    break;
	}

	case Reference::ModeBatchOneway:
	case Reference::ModeBatchDatagram:
	{
	    _emitter->prepareBatchRequest(this);
	    break;
	}
    }

    _os.write(_reference->identity);
    _os.write(_reference->facet);
    _os.write(operation);
    _os.write(Int(context.size()));
    Context::const_iterator p;
    for (p = context.begin(); p != context.end(); ++p)
    {
	_os.write(p->first);
	_os.write(p->second);
    }
    
    //
    // Input and output parameters are always sent in an
    // encapsulation, which makes it possible to forward oneway
    // requests as blobs.
    //
    _os.startWriteEncaps();
}

IceInternal::Outgoing::~Outgoing()
{
    if (_state == StateUnsent &&
	(_reference->mode == Reference::ModeBatchOneway || _reference->mode == Reference::ModeBatchDatagram))
    {
	_emitter->abortBatchRequest();
    }
}

bool
IceInternal::Outgoing::invoke()
{
    _os.endWriteEncaps();
    
    switch (_reference->mode)
    {
	case Reference::ModeTwoway:
	{
	    bool timedOut = false;
	    
	    {
		JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
		
		_emitter->sendRequest(this, false);
		_state = StateInProgress;
		
		Int timeout = _emitter->timeout();
		while (_state == StateInProgress)
		{
		    try
		    {
			if (timeout >= 0)
			{	
			    wait(timeout);
			    if (_state == StateInProgress)
			    {
				_state = StateLocalException;
				_exception = auto_ptr<LocalException>(new TimeoutException(__FILE__, __LINE__));
				timedOut = true;
			    }
			}
			else
			{
			    wait();
			}
		    }
		    catch(const JTCInterruptedException&)
		    {
		    }
		}
	    }
	    
	    if (_exception.get())
	    {
		if (timedOut)
		{
		    //
		    // Must be called outside the synchronization of this
		    // object
		    //
		    _emitter->exception(*_exception.get());
		}

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
	    
	    if (_state == StateException)
	    {
		return false;
	    }

	    if (_state == StateLocationForward)
	    {
		ObjectPrx p;
		_is.read(p);
		throw LocationForward(p);
	    }

	    assert(_state == StateOK);
	    break;
	}
	
	case Reference::ModeOneway:
	case Reference::ModeDatagram:
	{
	    _emitter->sendRequest(this, true);
	    _state = StateInProgress;
	    break;
	}

	case Reference::ModeBatchOneway:
	case Reference::ModeBatchDatagram:
	{
	    //
	    // The state must be set to StateInProgress before calling
	    // finishBatchRequest, because otherwise if
	    // finishBatchRequest raises an exception, the destructor
	    // of this class will call abortBatchRequest, and calling
	    // both finishBatchRequest and abortBatchRequest is
	    // illegal.
	    //
	    _state = StateInProgress;
	    _emitter->finishBatchRequest(this);
	    break;
	}
    }

    return true;
}

void
IceInternal::Outgoing::finished(BasicStream& is)
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    assert(_state != StateUnsent);
    if (_state == StateInProgress)
    {
	_is.swap(is);
	Byte status;
	_is.read(status);
	switch (static_cast<DispatchStatus>(status))
	{
	    case DispatchOK:
	    {
		//
		// Input and output parameters are always sent in an
		// encapsulation, which makes it possible to forward
		// oneway requests as blobs.
		//
		_is.startReadEncaps();
		_state = StateOK;
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
		_state = StateException;
		break;
	    }
	    
	    case DispatchLocationForward:
	    {
		_state = StateLocationForward;
		break;
	    }

	    case DispatchObjectNotExist:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new ObjectNotExistException(__FILE__, __LINE__));
		break;
	    }
	    
	    case DispatchFacetNotExist:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new FacetNotExistException(__FILE__, __LINE__));
		break;
	    }
	    
	    case DispatchOperationNotExist:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new OperationNotExistException(__FILE__, __LINE__));
		break;
	    }
	    
	    case DispatchUnknownLocalException:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new UnknownLocalException(__FILE__, __LINE__));
		break;
	    }

	    case DispatchUnknownUserException:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new UnknownUserException(__FILE__, __LINE__));
		break;
	    }

	    case DispatchUnknownException:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new UnknownException(__FILE__, __LINE__));
		break;
	    }
	    
	    default:
	    {
		_state = StateLocalException;
		_exception = auto_ptr<LocalException>(new UnknownReplyStatusException(__FILE__, __LINE__));
		break;
	    }
	}
	notify();
    }
}

void
IceInternal::Outgoing::finished(const LocalException& ex)
{
    JTCSyncT<JTCMonitorT<JTCMutex> > sync(*this);
    assert(_state != StateUnsent);
    if (_state == StateInProgress)
    {
	_state = StateLocalException;
	_exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	notify();
    }
}

BasicStream*
IceInternal::Outgoing::is()
{
    return &_is;
}

BasicStream*
IceInternal::Outgoing::os()
{
    return &_os;
}
