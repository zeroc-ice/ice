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
#include <Ice/ConnectionFactoryF.h>
#include <Ice/ObjectFactoryManagerF.h>
#include <Ice/UserExceptionFactoryManagerF.h>
#include <Ice/ObjectAdapterFactoryF.h>
#include <list>

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
    OutgoingConnectionFactoryPtr outgoingConnectionFactory();
    ObjectFactoryManagerPtr servantFactoryManager();
    UserExceptionFactoryManagerPtr userExceptionFactoryManager();
    ObjectAdapterFactoryPtr objectAdapterFactory();
    ThreadPoolPtr threadPool();
    std::string defaultProtocol();
    std::string defaultHost();
    
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
    OutgoingConnectionFactoryPtr _outgoingConnectionFactory;
    ObjectFactoryManagerPtr _servantFactoryManager;
    UserExceptionFactoryManagerPtr _userExceptionFactoryManager;
    ObjectAdapterFactoryPtr _objectAdapterFactory;
    ThreadPoolPtr _threadPool;
    std::string _defaultProtocol;
    std::string _defaultHost;

    //
    // Global state management
    //
    friend class GlobalStateMutexDestroyer;
    static int _globalStateCounter;
    static JTCMutex* _globalStateMutex;
    static JTCInitialize* _globalStateJTC;
#ifndef WIN32
    static std::string _identForOpenlog;
#endif
};

}

#endif
