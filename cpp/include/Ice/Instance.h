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

#include <Ice/InstanceF.h>
#include <Ice/CommunicatorF.h>
#include <Ice/PropertiesF.h>
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/ProxyFactoryF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/EmitterF.h>
#include <Ice/ValueFactoryManagerF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class Instance : public Shared, public JTCMutex
{
public:

    ::Ice::Communicator_ptr communicator();
    ::Ice::Properties_ptr properties();
    ::Ice::Logger_ptr logger();
    void logger(const ::Ice::Logger_ptr&);
    TraceLevels_ptr traceLevels();
    ProxyFactory_ptr proxyFactory();
    ThreadPool_ptr threadPool();
    EmitterFactory_ptr emitterFactory();
    ValueFactoryManager_ptr valueFactoryManager();
    
private:

    Instance(const ::Ice::Communicator_ptr&, const ::Ice::Properties_ptr&);
    virtual ~Instance();
    void destroy();
    friend class ::Ice::Communicator;

    ::Ice::Communicator_ptr communicator_;
    ::Ice::Properties_ptr properties_;
    ::Ice::Logger_ptr logger_;
    TraceLevels_ptr traceLevels_;
    ProxyFactory_ptr proxyFactory_;
    ThreadPool_ptr threadPool_;
    EmitterFactory_ptr emitterFactory_;
    ValueFactoryManager_ptr valueFactoryManager_;

    //
    // Global state management
    //
    friend class GlobalStateMutexDestroyer;
    static JTCMutex* globalStateMutex_;
    static JTCInitialize* globalStateJTC_;
    static int globalStateCounter_;
};

}

#endif
