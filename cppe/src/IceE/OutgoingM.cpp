// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Config.h>

#ifndef ICEE_PURE_BLOCKING_CLIENT

#include <IceE/Outgoing.h>
#include <IceE/DispatchStatus.h>
#include <IceE/Connection.h>
#include <IceE/Reference.h>
#include <IceE/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

IceInternal::OutgoingM::OutgoingM(Connection* connection, Reference* ref, const string& operation,
				  OperationMode mode, const Context& context) :
    Outgoing(connection, ref, operation, mode, context)
{
}

bool
IceInternal::OutgoingM::invoke()
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
	    _state = StateInProgress;
	    _connection->sendRequest(&_os, 0);
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
	    _state = StateInProgress;
	    _connection->finishBatchRequest(&_os);
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
IceInternal::OutgoingM::finished(BasicStream& is)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);

    assert(_reference->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);

    _is.swap(is);
    finishedInternal();
    notify();
}

void
IceInternal::OutgoingM::finished(const LocalException& ex)
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock sync(*this);
    
    assert(_reference->getMode() == Reference::ModeTwoway); // Can only be called for twoways.

    assert(_state <= StateInProgress);
    
    _state = StateLocalException;
    _exception.reset(dynamic_cast<LocalException*>(ex.ice_clone()));
     notify();
}

#endif
