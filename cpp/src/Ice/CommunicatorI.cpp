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
#include <Ice/PropertiesI.h>
#include <Ice/Instance.h>
#include <Ice/ProxyFactory.h>
#include <Ice/ThreadPool.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/ValueFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Logger.h>
#include <Ice/Initialize.h>
#include <Ice/LocalException.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::CommunicatorI::destroy()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    shutdown();
    _instance->destroy();
    _instance = 0;
}

void
Ice::CommunicatorI::shutdown()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->objectAdapterFactory()->shutdown();
}

void
Ice::CommunicatorI::waitForShutdown()
{
    ThreadPoolPtr threadPool;

    {
	JTCSyncT<JTCRecursiveMutex> sync(*this);
	if (!_instance)
	{
	    throw CommunicatorDestroyedException(__FILE__, __LINE__);
	}
	threadPool = _instance->threadPool();
    }

    threadPool->waitUntilServerFinished();
}

ObjectPrx
Ice::CommunicatorI::stringToProxy(const string& s)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->stringToProxy(s);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapter(const string& name)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    string endpts = _instance->properties()->getProperty("Ice.Adapter." + name + ".Endpoints");
    return createObjectAdapterWithEndpoints(name, endpts);
}

ObjectAdapterPtr
Ice::CommunicatorI::createObjectAdapterWithEndpoints(const string& name, const string& endpts)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->objectAdapterFactory() -> createObjectAdapter(name, endpts);
}

void
Ice::CommunicatorI::installValueFactory(const ValueFactoryPtr& factory, const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->valueFactoryManager()->install(factory, id);
}

PropertiesPtr
Ice::CommunicatorI::getProperties()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->properties();
}

LoggerPtr
Ice::CommunicatorI::getLogger()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->logger();
}

void
Ice::CommunicatorI::setLogger(const LoggerPtr& logger)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->logger(logger);
}

PicklerPtr
Ice::CommunicatorI::getPickler()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->pickler();
}

Ice::CommunicatorI::CommunicatorI(const PropertiesPtr& properties) :
    _instance(new ::IceInternal::Instance(this, properties))
{
}

Ice::CommunicatorI::~CommunicatorI()
{
    if (_instance)
    {
	_instance->logger()->warning("communicator object has not been destroyed");
    }
}

CommunicatorPtr
Ice::initialize(int&, char*[], Int version)
{
#ifndef ICE_IGNORE_VERSION
    if (version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    return new CommunicatorI(getDefaultProperties());
}

CommunicatorPtr
Ice::initializeWithProperties(int&, char*[], const PropertiesPtr& properties, Int version)
{
#ifndef ICE_IGNORE_VERSION
    if (version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    return new CommunicatorI(properties);
}

PropertiesPtr
Ice::getDefaultProperties()
{
    PropertiesPtr properties;
    const char* file = getenv("ICE_CONFIG");
    if (file && *file != '\0')
    {
	properties = new PropertiesI(file);
    }
    else
    {
	properties = new PropertiesI;
    }
    return properties;
}

PropertiesPtr
Ice::createProperties()
{
    return new PropertiesI;
}

PropertiesPtr
Ice::loadProperties(const std::string& file)
{
    return new PropertiesI(file);
}
