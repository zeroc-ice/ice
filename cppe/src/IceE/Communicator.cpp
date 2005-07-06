// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Communicator.h>
#include <IceE/Instance.h>
#include <IceE/Properties.h>
#include <IceE/ReferenceFactory.h>
#include <IceE/ProxyFactory.h>
#include <IceE/LoggerUtil.h>
#include <IceE/LocalException.h>
#ifndef ICEE_PURE_CLIENT
#    include <IceE/ObjectAdapterFactory.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
IceInternal::incRef(::Ice::Communicator* p)
{
    p->__incRef();
}

void
IceInternal::decRef(::Ice::Communicator* p)
{
    p->__decRef();
}

void
Ice::Communicator::destroy()
{
    InstancePtr instance;

    {
	RecMutex::Lock sync(*this);
	
	if(_destroyed) // Don't destroy twice.
	{
	    return;
	}
	_destroyed = true;
	instance = _instance;
    }

    if(instance)
    {
	instance->destroy();
    }
}

ObjectPrx
Ice::Communicator::stringToProxy(const string& s) const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::Communicator::proxyToString(const ObjectPrx& proxy) const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->proxyToString(proxy);
}

PropertiesPtr
Ice::Communicator::getProperties() const
{
    //
    // No check for destruction. It must be possible to access the
    // properties after destruction.
    //
    return _instance->properties();
}

LoggerPtr
Ice::Communicator::getLogger() const
{
    //
    // No check for destruction. It must be possible to access the
    // logger after destruction.
    //
    return _instance->logger();
}

void
Ice::Communicator::setLogger(const LoggerPtr& logger)
{
    //
    // No check for destruction. It must be possible to set the logger
    // after destruction (needed by logger plugins for example to
    // unset the logger).
    //
    _instance->logger(logger);
}

#ifndef ICEE_NO_ROUTER

RouterPrx
Ice::Communicator::getDefaultRouter() const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::Communicator::setDefaultRouter(const RouterPrx& router)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->referenceFactory()->setDefaultRouter(router);
}

#endif

#ifndef ICEE_NO_LOCATOR

LocatorPrx
Ice::Communicator::getDefaultLocator() const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::Communicator::setDefaultLocator(const LocatorPrx& locator)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->referenceFactory()->setDefaultLocator(locator);
}

#endif

void
Ice::Communicator::setDefaultContext(const Context& ctx)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->setDefaultContext(ctx);
}

Ice::Context
Ice::Communicator::getDefaultContext() const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->getDefaultContext();
}

#ifndef ICEE_NO_BATCH
void
Ice::Communicator::flushBatchRequests()
{
    _instance->flushBatchRequests();
}
#endif

Ice::Communicator::Communicator(const PropertiesPtr& properties) :
    _destroyed(false)
{
    __setNoDelete(true);
    try
    {
	_instance = new Instance(this, properties);
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

Ice::Communicator::~Communicator()
{
    RecMutex::Lock sync(*this);
    if(!_destroyed)
    {
	Warning out(_instance->logger());
	out << "Ice::Communicator::destroy() has not been called";
    }

    if(_instance->__getRef() > 1)
    {
	PropertiesPtr properties = _instance->properties();
	if(properties->getPropertyAsIntWithDefault("IceE.Warn.Leaks", 1) > 0)
	{
	    Warning warn(_instance->logger());
	    warn <<
		"The communicator is not the last Ice object that is\n"
		"deleted. (You can disable this warning by setting the\n"
		"property `IceE.Warn.Leaks' to 0.)";
	}
    }
}

void
Ice::Communicator::finishSetup(int& argc, char* argv[])
{
    _instance->finishSetup(argc, argv);
}

#ifndef ICEE_PURE_CLIENT

void
Ice::Communicator::shutdown()
{ 
    ObjectAdapterFactoryPtr objectAdapterFactory;

    {
	RecMutex::Lock sync(*this);
	if(_destroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}
	objectAdapterFactory = _instance->objectAdapterFactory();
    }

    //
    // We must call shutdown on the object adapter factory outside the
    // synchronization, otherwise the communicator is blocked during
    // shutdown.
    //
    objectAdapterFactory->shutdown();
}

void
Ice::Communicator::waitForShutdown()
{
    ObjectAdapterFactoryPtr objectAdapterFactory;

    {
	RecMutex::Lock sync(*this);
	if(_destroyed)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}
	objectAdapterFactory = _instance->objectAdapterFactory();
    }

    //
    // We must call waitForShutdown on the object adapter factory 
    // outside the synchronization, otherwise the communicator is
    // blocked during shutdown.
    //
    objectAdapterFactory->waitForShutdown();
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapter(const string& name)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    assert(_instance);
    ObjectAdapterPtr adapter = _instance->objectAdapterFactory()->createObjectAdapter(name);

    return adapter;
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    getProperties()->setProperty(name + ".Endpoints", endpoints);
    return createObjectAdapter(name);
}

#endif
