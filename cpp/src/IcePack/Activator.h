// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PACK_ACTIVATOR_H
#define ICE_PACK_ACTIVATOR_H

#include <IceUtil/Thread.h>

namespace IcePack
{

struct ServerDescription;

class Activator : public IceUtil::Thread, public IceUtil::Mutex
{
public:

    Activator(const Ice::CommunicatorPtr&);
    virtual ~Activator();

    virtual void run();
    void destroy();
    bool activate(const IcePack::ServerDescription&);

private:

    void terminationListener();
    void clearInterrupt();
    void setInterrupt();

    struct Process
    {
	pid_t pid;
	int fd;
    };

    Ice::CommunicatorPtr _communicator;
    std::map<std::string, Process> _processes;
    bool _destroy;
    int _fdIntrRead;
    int _fdIntrWrite;
};

typedef IceUtil::Handle<Activator> ActivatorPtr;

}

#endif
