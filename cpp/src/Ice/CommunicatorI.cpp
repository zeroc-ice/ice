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
Ice::initialize(int& argc, char* argv[], Int version)
{
#ifndef ICE_IGNORE_VERSION
    if (version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    return new CommunicatorI(getDefaultProperties(argc, argv));
}

CommunicatorPtr
Ice::initializeWithProperties(const PropertiesPtr& properties, Int version)
{
#ifndef ICE_IGNORE_VERSION
    if (version != ICE_INT_VERSION)
    {
	throw VersionMismatchException(__FILE__, __LINE__);
    }
#endif

    return new CommunicatorI(properties);
}

static PropertiesPtr defaultProperties;
class DefaultPropertiesDestroyer
{
public:

    ~DefaultPropertiesDestroyer()
    {
	defaultProperties = 0;
    }
};
static DefaultPropertiesDestroyer defaultPropertiesDestroyer;

PropertiesPtr
Ice::getDefaultProperties(int& argc, char* argv[])
{
    if (!defaultProperties)
    {
	defaultProperties = createProperties(argc, argv);
    }
    return defaultProperties;
}

PropertiesPtr
Ice::createProperties(int& argc, char* argv[])
{
    return new PropertiesI(argc, argv);
}

PropertiesPtr
Ice::createPropertiesFromFile(int& argc, char* argv[], const string& file)
{
    return new PropertiesI(argc, argv, file);
}
