// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <Ice/CommunicatorI.h>
#include <Ice/Instance.h>
#include <Ice/Properties.h>
#include <Ice/ReferenceFactory.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectFactoryManager.h>
#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/DefaultsAndOverrides.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::CommunicatorI::destroy()
{
    RecMutex::Lock sync(*this);

    if(!_destroyed) // Don't destroy twice.
    {
	_destroyed = true;
	_serverThreadPool = 0;
	_instance->destroy();
    }
}

void
Ice::CommunicatorI::shutdown()
{
    //
    // No mutex locking here! This operation must be signal-safe.
    //
    if(_serverThreadPool)
    {
	_serverThreadPool->initiateShutdown();
    }
}

void
Ice::CommunicatorI::waitForShutdown()
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
    // blocked while we wait for shutdown.
    //
    objectAdapterFactory->waitForShutdown();
}

ObjectPrx
Ice::CommunicatorI::stringToProxy(const string& s)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::CommunicatorI::proxyToString(const ObjectPrx& proxy)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->proxyToString(proxy);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapter(const string& name)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    
    ObjectAdapterPtr adapter;

    if(name.empty())
    {
	adapter = _instance->objectAdapterFactory()->createObjectAdapter("", "", "");
    }
    else
    {
	string id = _instance->properties()->getProperty(name + ".AdapterId");

	string endpts = _instance->properties()->getProperty(name + ".Endpoints");
	
	adapter = _instance->objectAdapterFactory()->createObjectAdapter(name, endpts, id);
	
	string router = _instance->properties()->getProperty(name + ".Router");
	if(!router.empty())
	{
	    adapter->addRouter(RouterPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(router)));
	}
	
	string locator = _instance->properties()->getProperty(name + ".Locator");
	if(!locator.empty())
	{
	    adapter->setLocator(LocatorPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(locator)));
	}
	else
	{
	    adapter->setLocator(_instance->referenceFactory()->getDefaultLocator());
	}
    }

    if(!_serverThreadPool) // Lazy initialization of _serverThreadPool.
    {
	_serverThreadPool = _instance->serverThreadPool();
    }

    return adapter;
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpoints)
{
    getProperties()->setProperty(name + ".Endpoints", endpoints);
    return createObjectAdapter(name);
}

void
Ice::CommunicatorI::addObjectFactory(const ObjectFactoryPtr& factory, const string& id)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->servantFactoryManager()->add(factory, id);
}

void
Ice::CommunicatorI::removeObjectFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->servantFactoryManager()->remove(id);
}

ObjectFactoryPtr
Ice::CommunicatorI::findObjectFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->servantFactoryManager()->find(id);
}

void
Ice::CommunicatorI::addUserExceptionFactory(const UserExceptionFactoryPtr& factory, const string& id)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->userExceptionFactoryManager()->add(factory, id);
}

void
Ice::CommunicatorI::removeUserExceptionFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->userExceptionFactoryManager()->remove(id);
}

UserExceptionFactoryPtr
Ice::CommunicatorI::findUserExceptionFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->userExceptionFactoryManager()->find(id);
}

PropertiesPtr
Ice::CommunicatorI::getProperties()
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->properties();
}

LoggerPtr
Ice::CommunicatorI::getLogger()
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->logger();
}

void
Ice::CommunicatorI::setLogger(const LoggerPtr& logger)
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->logger(logger);
}

void
Ice::CommunicatorI::setDefaultRouter(const RouterPrx& router)
{
    _instance->referenceFactory()->setDefaultRouter(router);
}

void
Ice::CommunicatorI::setDefaultLocator(const LocatorPrx& locator)
{
    _instance->referenceFactory()->setDefaultLocator(locator);
}

PluginManagerPtr
Ice::CommunicatorI::getPluginManager()
{
    RecMutex::Lock sync(*this);
    if(_destroyed)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->pluginManager();
}

Ice::CommunicatorI::CommunicatorI(int& argc, char* argv[], const PropertiesPtr& properties) :
    _destroyed(false)
{
    __setNoDelete(true);
    try
    {
	_instance = new Instance(this, argc, argv, properties);

        //
        // Keep a reference to the dynamic library list to ensure
        // the libraries are not unloaded until this Communicator's
        // destructor is invoked.
        //
        _dynamicLibraryList = _instance->dynamicLibraryList();
    }
    catch(...)
    {
	__setNoDelete(false);
	throw;
    }
    __setNoDelete(false);
}

Ice::CommunicatorI::~CommunicatorI()
{
    if(!_destroyed)
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
Ice::CommunicatorI::finishSetup(int& argc, char* argv[])
{
    _instance->finishSetup(argc, argv);
}
