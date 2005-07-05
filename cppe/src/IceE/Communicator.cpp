// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Object.h>
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
using namespace IceE;
using namespace IceEInternal;

void
IceEInternal::incRef(::IceE::Communicator* p)
{
    p->__incRef();
}

void
IceEInternal::decRef(::IceE::Communicator* p)
{
    p->__decRef();
}

void
IceE::Communicator::destroy()
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
IceE::Communicator::stringToProxy(const string& s) const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->stringToProxy(s);
}

string
IceE::Communicator::proxyToString(const ObjectPrx& proxy) const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->proxyToString(proxy);
}

PropertiesPtr
IceE::Communicator::getProperties() const
{
    //
    // No check for destruction. It must be possible to access the
    // properties after destruction.
    //
    return _instance->properties();
}

LoggerPtr
IceE::Communicator::getLogger() const
{
    //
    // No check for destruction. It must be possible to access the
    // logger after destruction.
    //
    return _instance->logger();
}

void
IceE::Communicator::setLogger(const LoggerPtr& logger)
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
IceE::Communicator::getDefaultRouter() const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->referenceFactory()->getDefaultRouter();
}

void
IceE::Communicator::setDefaultRouter(const RouterPrx& router)
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
IceE::Communicator::getDefaultLocator() const
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->referenceFactory()->getDefaultLocator();
}

void
IceE::Communicator::setDefaultLocator(const LocatorPrx& locator)
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
IceE::Communicator::setDefaultContext(const Context& ctx)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->setDefaultContext(ctx);
}

IceE::Context
IceE::Communicator::getDefaultContext() const
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
IceE::Communicator::flushBatchRequests()
{
    _instance->flushBatchRequests();
}
#endif

IceE::Communicator::Communicator(const PropertiesPtr& properties) :
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

IceE::Communicator::~Communicator()
{
    RecMutex::Lock sync(*this);
    if(!_destroyed)
    {
	Warning out(_instance->logger());
	out << "IceE::Communicator::destroy() has not been called";
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
IceE::Communicator::finishSetup(int& argc, char* argv[])
{
    _instance->finishSetup(argc, argv);
}

#ifndef ICEE_PURE_CLIENT

void
IceE::Communicator::shutdown()
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
IceE::Communicator::waitForShutdown()
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
IceE::Communicator::createObjectAdapter(const string& name)
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
IceE::Communicator::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    getProperties()->setProperty(name + ".Endpoints", endpoints);
    return createObjectAdapter(name);
}

#endif
