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
#include <Ice/CollectorF.h>
#include <Ice/Shared.h>

namespace __Ice
{

class InstanceI : public Shared, public JTCMutex
{
public:

    ::Ice::Communicator communicator();
    ::Ice::Properties properties();
    ::Ice::Logger logger();
    void logger(const ::Ice::Logger&);
    TraceLevels traceLevels();
    ProxyFactory proxyFactory();
    ThreadPool threadPool();
    EmitterFactory emitterFactory();
    
private:

    InstanceI(const ::Ice::Communicator&, const ::Ice::Properties&);
    virtual ~InstanceI();
    void destroy();
    friend class ::Ice::CommunicatorI; // May create and destroy InstanceIs

    ::Ice::Communicator communicator_;
    ::Ice::Properties properties_;
    ::Ice::Logger logger_;
    TraceLevels traceLevels_;
    ProxyFactory proxyFactory_;
    ThreadPool threadPool_;
    EmitterFactory emitterFactory_;

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
