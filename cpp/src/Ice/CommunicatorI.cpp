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
#include <Ice/ServantFactoryManager.h>
#include <Ice/UserExceptionFactoryManager.h>
#include <Ice/ObjectAdapterFactory.h>
#include <Ice/Logger.h>
#include <Ice/StreamI.h>
#include <Ice/Initialize.h>
#include <Ice/Exception.h>

using namespace std;
using namespace Ice;
using namespace IceInternal;

void
Ice::CommunicatorI::destroy()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);

    if (_instance)
    {
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
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    _threadPool->waitUntilServerFinished();
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

string
Ice::CommunicatorI::proxyToString(const ObjectPrx& proxy)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->proxyFactory()->proxyToString(proxy);
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
    return _instance->objectAdapterFactory()->createObjectAdapter(name, endpts);
}

void
Ice::CommunicatorI::addServantFactory(const ServantFactoryPtr& factory, const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->servantFactoryManager()->add(factory, id);
}

void
Ice::CommunicatorI::removeServantFactory(const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->servantFactoryManager()->remove(id);
}

ServantFactoryPtr
Ice::CommunicatorI::findServantFactory(const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->servantFactoryManager()->find(id);
}

void
Ice::CommunicatorI::addUserExceptionFactory(const UserExceptionFactoryPtr& factory, const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->userExceptionFactoryManager()->add(factory, id);
}

void
Ice::CommunicatorI::removeUserExceptionFactory(const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    _instance->userExceptionFactoryManager()->remove(id);
}

UserExceptionFactoryPtr
Ice::CommunicatorI::findUserExceptionFactory(const string& id)
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }
    return _instance->userExceptionFactoryManager()->find(id);
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

StreamPtr
Ice::CommunicatorI::createStream()
{
    JTCSyncT<JTCRecursiveMutex> sync(*this);
    if (!_instance)
    {
	throw CommunicatorDestroyedException(__FILE__, __LINE__);
    }

    return new StreamI(_instance);
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
    // Se the comments in the header file for an explanation of why we
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

InstancePtr
IceInternal::getInstance(const CommunicatorPtr& communicator)
{
    CommunicatorI* p = dynamic_cast<CommunicatorI*>(communicator.get());
    assert(p);
    return p->_instance;
}
