// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceE/Communicator.h>
#ifdef ICEE_HAS_ROUTER
#  include <IceE/Router.h>
#endif
#ifdef ICEE_HAS_LOCATOR
#  include <IceE/Locator.h>
#endif
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

IceUtil::Shared* IceInternal::upCast(::Ice::Communicator* p) { return p; }

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

bool
Ice::Communicator::isShutdown() const
{
    return _instance->objectAdapterFactory()->isShutdown();
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

ObjectPrx
Ice::Communicator::propertyToProxy(const string& p) const
{
    return _instance->proxyFactory()->propertyToProxy(p);
}

Identity
Ice::Communicator::stringToIdentity(const string& s) const
{
    return _instance->stringToIdentity(s);
}

string
Ice::Communicator::identityToString(const Identity& ident) const
{
    return _instance->identityToString(ident);
}

#ifndef ICEE_PURE_CLIENT

ObjectAdapterPtr
Ice::Communicator::createObjectAdapter(const string& name)
{
    return createObjectAdapterWithEndpoints(name, getProperties()->getProperty(name + ".Endpoints"));
}

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, endpoints
#ifdef ICEE_HAS_ROUTER
								  , 0
# endif
    								  );
}

#ifdef ICEE_HAS_ROUTER

ObjectAdapterPtr
Ice::Communicator::createObjectAdapterWithRouter(const string& name, const RouterPrx& router)
{
    return _instance->objectAdapterFactory()->createObjectAdapter(name, "", router);
}

#endif

#endif

PropertiesPtr
Ice::Communicator::getProperties() const
{
    return _instance->initializationData().properties;
}

LoggerPtr
Ice::Communicator::getLogger() const
{
    return _instance->initializationData().logger;
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

Ice::Communicator::Communicator(const InitializationData& initData) 
{
    __setNoDelete(true);
    try
    {
	const_cast<InstancePtr&>(_instance) = new Instance(this, initData);
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
	Warning out(_instance->initializationData().logger);
	out << "Ice::Communicator::destroy() has not been called";
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
