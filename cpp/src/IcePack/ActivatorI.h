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

class ActivatorI : public Activator, public IceUtil::Thread, public IceUtil::Mutex
{
public:

    ActivatorI(const Ice::CommunicatorPtr&, const std::vector<std::string>&);
    virtual ~ActivatorI();

    virtual void run();
    void destroy();

    virtual bool activate(const ::IcePack::ServerPrx&, const ::Ice::Current&);

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
    std::vector<Process> _processes;
    bool _destroy;
    int _fdIntrRead;
    int _fdIntrWrite;
    std::vector<std::string> _defaultArgs;
};

typedef IceUtil::Handle<ActivatorI> ActivatorIPtr;

}

#endif
