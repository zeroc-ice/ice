// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_INSTANCE_H
#define ICE_INSTANCE_H

#include <IceUtil/Shared.h>
#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/EmitterF.h>
#include <Ice/ServantFactoryManagerF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <Ice/PicklerF.h>

namespace Ice
{

class CommunicatorI;

}

namespace IceInternal
{

class Instance : public ::IceUtil::Shared, public JTCMutex
{
public:

    ::Ice::CommunicatorPtr communicator();
    ::Ice::PropertiesPtr properties();
    ::Ice::LoggerPtr logger();
    void logger(const ::Ice::LoggerPtr&);
    TraceLevelsPtr traceLevels();
    ProxyFactoryPtr proxyFactory();
    ThreadPoolPtr threadPool();
    EmitterFactoryPtr emitterFactory();
    ServantFactoryManagerPtr valueFactoryManager();
    ObjectAdapterFactoryPtr objectAdapterFactory();
    ::Ice::PicklerPtr pickler();
    
private:

    Instance(const ::Ice::CommunicatorPtr&, const ::Ice::PropertiesPtr&);
    virtual ~Instance();
    void destroy();
    friend class ::Ice::CommunicatorI;

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::PropertiesPtr _properties;
    ::Ice::LoggerPtr _logger;
    TraceLevelsPtr _traceLevels;
    ProxyFactoryPtr _proxyFactory;
    ThreadPoolPtr _threadPool;
    EmitterFactoryPtr _emitterFactory;
    ServantFactoryManagerPtr _valueFactoryManager;
    ObjectAdapterFactoryPtr _objectAdapterFactory;
    ::Ice::PicklerPtr _pickler;

    //
    // Global state management
    //
    friend class GlobalStateMutexDestroyer;
    static JTCMutex* _globalStateMutex;
    static JTCInitialize* _globalStateJTC;
#ifndef WIN32
    static std::string _identForOpenlog;
#endif
    static int _globalStateCounter;
};

}

#endif
