// **********************************************************************
//
// Copyright (c) 2003
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
    void clearInterrupt();
    void setInterrupt();

    struct Process
    {
#ifdef _WIN32
        DWORD pid;
        HANDLE hnd;
#else
	pid_t pid;
	int fd;
#endif
	ServerPtr server;
    };

    TraceLevelsPtr _traceLevels;
    Ice::PropertiesPtr _properties;
    std::vector<Process> _processes;
    bool _deactivating;

#ifdef _WIN32
    HANDLE _hIntr;
#else
    int _fdIntrRead;
    int _fdIntrWrite;
#endif

    std::vector<std::string> _propertiesOverride;
    
    IceUtil::ThreadPtr _thread;
};

}

#endif
