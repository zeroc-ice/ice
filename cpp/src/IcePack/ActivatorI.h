// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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

    ActivatorI(const TraceLevelsPtr&);
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
    std::vector<Process> _processes;
    bool _deactivating;

    int _fdIntrRead;
    int _fdIntrWrite;

    IceUtil::ThreadPtr _thread;
};

typedef IceUtil::Handle<ActivatorI> ActivatorIPtr;

}

#endif
