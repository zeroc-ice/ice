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
#include <IcePack/ServerManagerF.h>

namespace IcePack
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ActivatorI : public Activator, public IceUtil::Thread, public IceUtil::Mutex
{
public:

    ActivatorI(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    virtual ~ActivatorI();

    virtual void run();

    virtual Ice::Int activate(const ::IcePack::ServerPrx&);
    virtual void deactivate(const ::IcePack::ServerPrx&);
    virtual void kill(const ::IcePack::ServerPrx&);
    virtual void destroy();

private:

    void terminationListener();
    void clearInterrupt();
    void setInterrupt();

    struct Process
    {
	pid_t pid;
	int fd;
	ServerPrx server;
    };

    Ice::CommunicatorPtr _communicator;
    TraceLevelsPtr _traceLevels;
    std::vector<Process> _processes;
    bool _destroy;
    bool _deactivating;

    int _fdIntrRead;
    int _fdIntrWrite;
};

typedef IceUtil::Handle<ActivatorI> ActivatorIPtr;

}

#endif
