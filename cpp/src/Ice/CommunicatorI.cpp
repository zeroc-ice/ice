// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
#include <Ice/Logger.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::CommunicatorI::destroy()
{
    RecMutex::Lock sync(*this);

    if (_instance)
    {
	_instance->objectAdapterFactory()->shutdown();
	_instance->destroy();
	_instance = 0;
    }
}

void
Ice::CommunicatorI::shutdown()
{
    //
    // No mutex locking here! This operation must be signal-safe.
    //
    _threadPool->initiateServerShutdown();
}

void
Ice::CommunicatorI::waitForShutdown()
{
    //
    // No mutex locking here, otherwise the communicator is blocked
    // while waiting for shutdown.
    //
    _threadPool->waitUntilServerFinished();
}

ObjectPrx
Ice::CommunicatorI::stringToProxy(const string& s)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->stringToProxy(s);
}

string
Ice::CommunicatorI::proxyToString(const ObjectPrx& proxy)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->proxyToString(proxy);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapter(const string& name)
{
    ObjectAdapterPtr adapter = createObjectAdapterFromProperty(name, "Ice.Adapter." + name + ".Endpoints");
    string router = _instance->properties()->getProperty("Ice.Adapter." + name + ".Router");
    if (!router.empty())
    {
	adapter->addRouter(RouterPrx::uncheckedCast(_instance->proxyFactory()->stringToProxy(router)));
    }
    return adapter;
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterFromProperty(const string& name, const string& property)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    string endpts = _instance->properties()->getProperty(property);
    return createObjectAdapterWithEndpoints(name, endpts);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpts)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->objectAdapterFactory()->createObjectAdapter(name, endpts);
}

void
Ice::CommunicatorI::addObjectFactory(const ObjectFactoryPtr& factory, const string& id)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->servantFactoryManager()->add(factory, id);
}

void
Ice::CommunicatorI::removeObjectFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->servantFactoryManager()->remove(id);
}

ObjectFactoryPtr
Ice::CommunicatorI::findObjectFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->servantFactoryManager()->find(id);
}

void
Ice::CommunicatorI::addUserExceptionFactory(const UserExceptionFactoryPtr& factory, const string& id)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->userExceptionFactoryManager()->add(factory, id);
}

void
Ice::CommunicatorI::removeUserExceptionFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->userExceptionFactoryManager()->remove(id);
}

UserExceptionFactoryPtr
Ice::CommunicatorI::findUserExceptionFactory(const string& id)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->userExceptionFactoryManager()->find(id);
}

PropertiesPtr
Ice::CommunicatorI::getProperties()
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->properties();
}

LoggerPtr
Ice::CommunicatorI::getLogger()
{
    RecMutex::Lock sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->logger();
}

void
Ice::CommunicatorI::setLogger(const LoggerPtr& logger)
{
    RecMutex::Lock sync(*this);
    if (!_instance)
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

Ice::CommunicatorI::CommunicatorI(const PropertiesPtr& properties)
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

    //
    // See the comments in the header file for an explanation of why we
    // need _threadPool directly in CommunicatorI.
    //
    _threadPool = _instance->threadPool();
}

Ice::CommunicatorI::~CommunicatorI()
{
    if (_instance)
    {
	_instance->logger()->warning("communicator has not been destroyed");
    }
}
