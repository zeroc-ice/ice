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
#include <Ice/LoggerF.h>
#include <Ice/TraceLevelsF.h>
#include <Ice/ObjectFactoryF.h>
#include <Ice/ThreadPoolF.h>
#include <Ice/EmitterF.h>
#include <Ice/CollectorF.h>
#include <Ice/Shared.h>

namespace _Ice
{

class ICE_API InstanceI : public Shared, public JTCMutex
{
public:

    ::IceLocal::Logger logger();
    void logger(const ::IceLocal::Logger&);
    TraceLevels traceLevels();
    ObjectFactory objectFactory();
    ThreadPool threadPool();
    EmitterFactoryFactory emitterFactoryFactory();
    CollectorFactoryFactory collectorFactoryFactory();
    
private:

    InstanceI(const InstanceI&);
    void operator=(const InstanceI&);

    InstanceI();
    virtual ~InstanceI();
    void destroy();
    friend class ::Ice::CommunicatorI; // May create and destroy InstanceIs

    ::IceLocal::Logger logger_;
    TraceLevels traceLevels_;
    ObjectFactory objectFactory_;
    ThreadPool threadPool_;
    EmitterFactoryFactory emitterFactoryFactory_;
    CollectorFactoryFactory collectorFactoryFactory_;

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
