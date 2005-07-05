// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/IncomingConnectionFactory.h>
#include <IceE/Connection.h>
#include <IceE/Instance.h>
#include <IceE/LoggerUtil.h>
#include <IceE/TraceLevels.h>
#include <IceE/DefaultsAndOverrides.h>
#include <IceE/Properties.h>
#include <IceE/Transceiver.h>
#include <IceE/Acceptor.h>
#include <IceE/Endpoint.h>
#include <IceE/LocalException.h>
#include <IceE/Functional.h>

using namespace std;
using namespace IceE;
using namespace IceEInternal;

void IceEInternal::incRef(IncomingConnectionFactory* p) { p->__incRef(); }
void IceEInternal::decRef(IncomingConnectionFactory* p) { p->__decRef(); }

void
IceEInternal::IncomingConnectionFactory::activate()
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);
    setState(StateActive);
}

void
IceEInternal::IncomingConnectionFactory::hold()
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);
    setState(StateHolding);
}

void
IceEInternal::IncomingConnectionFactory::destroy()
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);
    setState(StateClosed);
}

void
IceEInternal::IncomingConnectionFactory::waitUntilHolding() const
{
    list<ConnectionPtr> connections;

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	
	//
	// First we wait until the connection factory itself is in holding
	// state.
	//
	while(_state < StateHolding)
	{
	    wait();
	}

	//
	// We want to wait until all connections are in holding state
	// outside the thread synchronization.
	//
	connections = _connections;
    }

    //
    // Now we wait until each connection is in holding state.
    //
    for_each(connections.begin(), connections.end(), IceE::constVoidMemFun(&Connection::waitUntilHolding));
}

void
IceEInternal::IncomingConnectionFactory::waitUntilFinished()
{
    IceE::ThreadPtr threadPerIncomingConnectionFactory;
    list<ConnectionPtr> connections;

    {
	IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	
	//
	// First we wait until the factory is destroyed. If we are using
	// an acceptor, we also wait for it to be closed.
	//
	while(_state != StateClosed || _acceptor)
	{
	    wait();
	}

	threadPerIncomingConnectionFactory = _threadPerIncomingConnectionFactory;
	_threadPerIncomingConnectionFactory = 0;

	//
	// We want to wait until all connections are finished outside the
	// thread synchronization.
	//
	connections.swap(_connections);
    }

    if(threadPerIncomingConnectionFactory)
    {
	threadPerIncomingConnectionFactory->getThreadControl().join();
    }

    for_each(connections.begin(), connections.end(), IceE::voidMemFun(&Connection::waitUntilFinished));
}

EndpointPtr
IceEInternal::IncomingConnectionFactory::endpoint() const
{
    // No mutex protection necessary, _endpoint is immutable.
    return _endpoint;
}

bool
IceEInternal::IncomingConnectionFactory::equivalent(const EndpointPtr& endp) const
{
    if(_transceiver)
    {
	return endp->equivalent(_transceiver);
    }
    
    assert(_acceptor);
    return endp->equivalent(_acceptor);
}

list<ConnectionPtr>
IceEInternal::IncomingConnectionFactory::connections() const
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);

    list<ConnectionPtr> result;

    //
    // Only copy connections which have not been destroyed.
    //
    remove_copy_if(_connections.begin(), _connections.end(), back_inserter(result),
		   IceE::constMemFun(&Connection::isDestroyed));

    return result;
}

void
IceEInternal::IncomingConnectionFactory::flushBatchRequests()
{
    list<ConnectionPtr> c = connections(); // connections() is synchronized, so no need to synchronize here.

    for(list<ConnectionPtr>::const_iterator p = c.begin(); p != c.end(); ++p)
    {
	try
	{
	    (*p)->flushBatchRequests();
	}
	catch(const LocalException&)
	{
	    // Ignore.
	}
    }
}

string
IceEInternal::IncomingConnectionFactory::toString() const
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);

    if(_transceiver)
    {
	return _transceiver->toString();
    }
    
    assert(_acceptor);
    return _acceptor->toString();
}

IceEInternal::IncomingConnectionFactory::IncomingConnectionFactory(const InstancePtr& instance,
								  const EndpointPtr& endpoint,
								  const ObjectAdapterPtr& adapter) :
    _instance(instance),
    _endpoint(endpoint),
    _adapter(adapter),
    _warn(_instance->properties()->getPropertyAsInt("IceE.Warn.Connections") > 0),
    _state(StateHolding)
{
    if(_instance->defaultsAndOverrides()->overrideTimeout)
    {
	const_cast<EndpointPtr&>(_endpoint) =
	    _endpoint->timeout(_instance->defaultsAndOverrides()->overrideTimeoutValue);
    }

    _acceptor = _endpoint->acceptor(const_cast<EndpointPtr&>(_endpoint));
    assert(_acceptor);
    _acceptor->listen();
    
    __setNoDelete(true);
    try
    {
	//
	// If we are in thread per connection mode, we also use one
	// thread per incoming connection factory, that accepts new
	// connections on this endpoint.
	//
	_threadPerIncomingConnectionFactory = new ThreadPerIncomingConnectionFactory(this);
	_threadPerIncomingConnectionFactory->start(_instance->threadPerConnectionStackSize());
    }
    catch(const IceE::Exception& ex)
    {
	{
	    Error out(_instance->logger());
	    out << "cannot create thread for incoming connection factory:\n" << ex.toString();
	}
	
	try
	{
	    _acceptor->close();
	}
	catch(const LocalException&)
	{
	    // Here we ignore any exceptions in close().
	}
	
	_state = StateClosed;
	_acceptor = 0;
	_threadPerIncomingConnectionFactory = 0;
	
	__setNoDelete(false);
	ex.ice_throw();
    }
    __setNoDelete(false);
}

IceEInternal::IncomingConnectionFactory::~IncomingConnectionFactory()
{
    IceE::Monitor<IceE::Mutex>::Lock sync(*this);
    
    assert(_state == StateClosed);
    assert(!_acceptor);
    assert(_connections.empty());
    assert(!_threadPerIncomingConnectionFactory);
}

void
IceEInternal::IncomingConnectionFactory::setState(State state)
{
    if(_state == state) // Don't switch twice.
    {
	return;
    }

    switch(state)
    {
	case StateActive:
	{
	    if(_state != StateHolding) // Can only switch from holding to active.
	    {
		return;
	    }
	    for_each(_connections.begin(), _connections.end(), IceE::voidMemFun(&Connection::activate));
	    break;
	}
	
	case StateHolding:
	{
	    if(_state != StateActive) // Can only switch from active to holding.
	    {
		return;
	    }
	    for_each(_connections.begin(), _connections.end(), IceE::voidMemFun(&Connection::hold));
	    break;
	}
	
	case StateClosed:
	{
	    if(_acceptor)
	    {
	        //
	        // Connect to our own acceptor, which unblocks our
	        // thread per incoming connection factory stuck in accept().
	        //
	        _acceptor->connectToSelf();
	    }

#ifdef _STLP_BEGIN_NAMESPACE
	    // voidbind2nd is an STLport extension for broken compilers in IceE/Functional.h
	    for_each(_connections.begin(), _connections.end(),
		     voidbind2nd(IceE::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
#else
	    for_each(_connections.begin(), _connections.end(),
		     bind2nd(IceE::voidMemFun1(&Connection::destroy), Connection::ObjectAdapterDeactivated));
#endif
	    break;
	}
    }

    _state = state;
    notifyAll();
}

void
IceEInternal::IncomingConnectionFactory::run()
{
    assert(_acceptor);

    while(true)
    {
	//
	// We must accept new connections outside the thread
	// synchronization, because we use blocking accept.
	//
	TransceiverPtr transceiver;
	try
	{
	    transceiver = _acceptor->accept(-1);
	}
	catch(const SocketException&)
	{
	    // Ignore socket exceptions.
	}
	catch(const TimeoutException&)
	{
	    // Ignore timeouts.
	}
	catch(const LocalException& ex)
	{
	    // Warn about other Ice local exceptions.
	    if(_warn)
	    {
		Warning out(_instance->logger());
		out << "connection exception:\n" << ex.toString() << "\n" << _acceptor->toString();
	    }
	}
	
	ConnectionPtr connection;
	
	{
	    IceE::Monitor<IceE::Mutex>::Lock sync(*this);
	    
	    while(_state == StateHolding)
	    {
		wait();
	    }
	    
	    if(_state == StateClosed)
	    {
		if(transceiver)
		{
		    try
		    {
			transceiver->close();
		    }
		    catch(const LocalException&)
		    {
			// Here we ignore any exceptions in close().
		    }
		}
		
		try
		{
		    _acceptor->close();
		}
		catch(const LocalException& ex)
		{
		    _acceptor = 0;
		    notifyAll();
		    ex.ice_throw();
		}
		
		_acceptor = 0;
		notifyAll();
		return;
	    }
	    
	    assert(_state == StateActive);
	    
	    //
	    // Reap connections for which destruction has completed.
	    //
	    _connections.erase(remove_if(_connections.begin(), _connections.end(),
					 IceE::constMemFun(&Connection::isFinished)),
			       _connections.end());
	    
	    //
	    // Create a connection object for the connection.
	    //
	    if(transceiver)
	    {
		try
		{
		    connection = new Connection(_instance, transceiver, _endpoint, _adapter);
		}
		catch(const LocalException&)
		{
		    return;
		}

		_connections.push_back(connection);
	    }
	}
	
	//
	// In thread per connection mode, the connection's thread will
	// take care of connection validation and activation. We don't
	// want to block this thread waiting until validation is
	// complete because it is the only thread that can accept
	// connections with this factory's acceptor. Therefore we
	// don't call validate() and activate() from the connection
	// factory in thread per connection mode.
	//
    }
}

IceEInternal::IncomingConnectionFactory::ThreadPerIncomingConnectionFactory::ThreadPerIncomingConnectionFactory(
    const IncomingConnectionFactoryPtr& factory) :
    _factory(factory)
{
}

void
IceEInternal::IncomingConnectionFactory::ThreadPerIncomingConnectionFactory::run()
{
    try
    {
	_factory->run();
    }
    catch(const Exception& ex)
    {	
	Error out(_factory->_instance->logger());
	out << "exception in thread per incoming connection factory:\n" << _factory->toString() << ex.toString(); 
    }
    catch(const std::exception& ex)
    {
	Error out(_factory->_instance->logger());
	out << "std::exception in thread per incoming connection factory:\n" << _factory->toString() << ex.what();
    }
    catch(...)
    {
	Error out(_factory->_instance->logger());
	out << "unknown exception in thread per incoming connection factory:\n" << _factory->toString();
    }

    _factory = 0; // Resolve cyclic dependency.
}
