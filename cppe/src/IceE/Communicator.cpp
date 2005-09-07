// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
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
#   include <IceE/ObjectAdapterFactory.h>
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
    _instance->destroy();
}

#ifndef ICEE_PURE_CLIENT

void
Ice::Communicator::shutdown()
{ 
    _instance->objectAdapterFactory()->shutdown();
}

void
Ice::Communicator::waitForShutdown()
{
    _instance->objectAdapterFactory()->waitForShutdown();
}
#endif

ObjectPrx
Ice::Communicator::stringToProxy(const string& s) const
{
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::Communicator::proxyToString(const ObjectPrx& proxy) const
{
    return _instance->proxyFactory()->proxyToString(proxy);
}

#ifndef ICEE_PURE_CLIENT

ObjectAdapterPtr
Ice::Communicator::createObjectAdapter(const string& name)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name);
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    const string propertyKey = name + ".Endpoints";
    const string originalValue = getProperties()->getProperty(propertyKey);
    try
    {
	getProperties()->setProperty(propertyKey, endpoints);
	return createObjectAdapter(name);
    }
    catch(const AlreadyRegisteredException&)
    {
	getProperties()->setProperty(propertyKey, originalValue);
	throw;
    }
}

#endif

void
Ice::Communicator::setDefaultContext(const Context& ctx)
{
    _instance->setDefaultContext(ctx);
}

Ice::Context
Ice::Communicator::getDefaultContext() const
{
    return _instance->getDefaultContext();
}

PropertiesPtr
Ice::Communicator::getProperties() const
{
    return _instance->properties();
}

LoggerPtr
Ice::Communicator::getLogger() const
{
    return _instance->logger();
}

void
Ice::Communicator::setLogger(const LoggerPtr& logger)
{
    _instance->logger(logger);
}

#ifdef ICEE_HAS_ROUTER

RouterPrx
Ice::Communicator::getDefaultRouter() const
{
    return _instance->referenceFactory()->getDefaultRouter();
}

void
Ice::Communicator::setDefaultRouter(const RouterPrx& router)
{
    _instance->referenceFactory()->setDefaultRouter(router);
}

#endif

#ifdef ICEE_HAS_LOCATOR

LocatorPrx
Ice::Communicator::getDefaultLocator() const
{
    return _instance->referenceFactory()->getDefaultLocator();
}

void
Ice::Communicator::setDefaultLocator(const LocatorPrx& locator)
{
    _instance->referenceFactory()->setDefaultLocator(locator);
}

#endif

#ifdef ICEE_HAS_BATCH

void
Ice::Communicator::flushBatchRequests()
{
    _instance->flushBatchRequests();
}

#endif

Ice::Communicator::Communicator(const PropertiesPtr& properties) 
{
    __setNoDelete(true);
    try
    {
	const_cast<InstancePtr&>(_instance) = new Instance(this, properties);
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
    if(!_instance->destroyed())
    {
	Warning out(_instance->logger());
	out << "Ice::Communicator::destroy() has not been called";
    }

    if(_instance->__getRef() > 1)
    {
	PropertiesPtr properties = _instance->properties();
	if(properties->getPropertyAsIntWithDefault("Ice.Warn.Leaks", 1) > 0)
	{
	    Warning warn(_instance->logger());
	    warn <<
		"The communicator is not the last Ice object that is\n"
		"deleted. (You can disable this warning by setting the\n"
		"property `Ice.Warn.Leaks' to 0.)";
	}
    }
}

void
Ice::Communicator::finishSetup(int& argc, char* argv[])
{
    try
    {
	_instance->finishSetup(argc, argv);
    }
    catch(...)
    {
	_instance->destroy();
	throw;
    }
}
