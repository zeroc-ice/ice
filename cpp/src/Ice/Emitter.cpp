// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Emitter.h>
#include <Ice/Instance.h>
#include <Ice/Logger.h>
#include <Ice/TraceLevels.h>
#include <Ice/Transceiver.h>
#include <Ice/Connector.h>
#include <Ice/Connection.h>
#include <Ice/Endpoint.h>
#include <Ice/Exception.h>
#include <Ice/Functional.h>
#include <Ice/SecurityException.h> // TODO: bandaid, see below.

using namespace std;
using namespace Ice;
using namespace IceInternal;

void IceInternal::incRef(EmitterFactory* p) { p->__incRef(); }
void IceInternal::decRef(EmitterFactory* p) { p->__decRef(); }

IceInternal::EmitterFactory::EmitterFactory(const InstancePtr& instance) :
    _instance(instance)
{
}

IceInternal::EmitterFactory::~EmitterFactory()
{
    assert(!_instance);
}

ConnectionPtr
IceInternal::EmitterFactory::create(const vector<EndpointPtr>& endpoints)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    assert(!endpoints.empty());

    //
    // First reap destroyed connections
    //
    std::map<EndpointPtr, ConnectionPtr>::iterator p = _connections.begin();
    while (p != _connections.end())
    {
	if (p->second->destroyed())
	{
	    std::map<EndpointPtr, ConnectionPtr>::iterator p2 = p;	    
	    ++p;
	    _connections.erase(p2);
	}
	else
	{
	    ++p;
	}
    }

    //
    // Search for existing connections
    //
    vector<EndpointPtr>::const_iterator q;
    for (q = endpoints.begin(); q != endpoints.end(); ++q)
    {
	map<EndpointPtr, ConnectionPtr>::const_iterator r = _connections.find(*q);
	if (r != _connections.end())
	{
	    return r->second;
	}
    }

    //
    // No connections exist, try to create one
    //
    TraceLevelsPtr traceLevels = _instance->traceLevels();
    LoggerPtr logger = _instance->logger();

    ConnectionPtr connection;
    auto_ptr<LocalException> exception;
    q = endpoints.begin();
    while (q != endpoints.end())
    {
	try
	{
	    TransceiverPtr transceiver = (*q)->clientTransceiver();
	    if (!transceiver)
	    {
		ConnectorPtr connector = (*q)->connector();
		assert(connector);
		transceiver = connector->connect((*q)->timeout());
		assert(transceiver);
	    }	    
	    connection = new Connection(_instance, transceiver, *q, 0);
	    connection->activate();
	    _connections.insert(make_pair(*q, connection));
	    break;
	}
	catch (const SocketException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch (const IceSecurity::SecurityException& ex) // TODO: bandaid to make retry w/ ssl work.
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch (const DNSException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}
	catch (const TimeoutException& ex)
	{
	    exception = auto_ptr<LocalException>(dynamic_cast<LocalException*>(ex.ice_clone()));
	}

	++q;

	if (traceLevels->retry >= 2)
	{
	    ostringstream s;
	    s << "connection to endpoint failed";
	    if (q != endpoints.end())
	    {
		s << ", trying next endpoint\n";
	    }
	    else
	    {
		s << " and no more endpoints to try\n";
	    }
	    s << *exception.get();
	    logger->trace(traceLevels->retryCat, s.str());
	}
    }

    if (!connection)
    {
	assert(exception.get());
	exception->ice_throw();
    }

    return connection;
}

void
IceInternal::EmitterFactory::destroy()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (!_instance)
    {
	return;
    }

    // voidbind2nd is an STLport extension for broken compilers in IceUtil/Functional.h
    for_each(_connections.begin(), _connections.end(),
	     voidbind2nd(Ice::secondVoidMemFun1<EndpointPtr, Connection, Connection::DestructionReason>
			 (&Connection::destroy), Connection::CommunicatorDestroyed));
    _connections.clear();
    _instance = 0;
}
