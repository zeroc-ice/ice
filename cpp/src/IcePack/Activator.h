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

#include <IcePack/AdminF.h>
#include <map>

namespace IcePack
{

class Activator : public JTCThread, public JTCMutex
{
public:

    Activator(const Ice::CommunicatorPtr&);
    virtual ~Activator();

    virtual void run();
    void destroy();
    void activate(const IcePack::ServerDescriptionPtr&);

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

typedef JTCHandleT<Activator> ActivatorHandle;

}

#endif
