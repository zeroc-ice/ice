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

#ifndef ICE_PACK_ACTIVATOR_H
#define ICE_PACK_ACTIVATOR_H

#include <IceUtil/Thread.h>
#include <IcePack/Activator.h>
#include <IcePack/Internal.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ActivatorI : public Activator, public IceUtil::Monitor< IceUtil::Mutex>
{
public:

    ActivatorI(const TraceLevelsPtr&, const Ice::PropertiesPtr&);
    virtual ~ActivatorI();

    virtual bool activate(const ::IcePack::ServerPtr&);
    virtual void deactivate(const ::IcePack::ServerPtr&);
    virtual void kill(const ::IcePack::ServerPtr&);
    virtual Ice::Int getServerPid(const ::IcePack::ServerPtr&);
    
    virtual void start();
    virtual void waitForShutdown();
    virtual void shutdown();
    virtual void destroy();
    
    void runTerminationListener();

private:

    void deactivateAll();    

    void terminationListener();
    bool clearInterrupt();
    void setInterrupt(char);

    struct Process
    {
	pid_t pid;
	int fd;
	ServerPtr server;
    };

    TraceLevelsPtr _traceLevels;
    Ice::PropertiesPtr _properties;
    std::vector<Process> _processes;
    bool _deactivating;

    int _fdIntrRead;
    int _fdIntrWrite;

    IceUtil::ThreadPtr _thread;
};

}

#endif
