// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
    virtual void sendSignal(const ::IcePack::ServerPtr&, const std::string&);
    virtual void writeMessage(const ::IcePack::ServerPtr&, const std::string&, Ice::Int);

    virtual Ice::Int getServerPid(const ::IcePack::ServerPtr&);
    
    virtual void start();
    virtual void waitForShutdown();
    virtual void shutdown();
    virtual void destroy();
    
    
    void sendSignal(const ::IcePack::ServerPtr&, int);
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
	HANDLE outHandle;
	HANDLE errHandle;
#else
	pid_t pid;
	int pipeFd;
	int outFd;
	int errFd;
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

    std::string _outputDir;
    bool _redirectErrToOut;

    IceUtil::ThreadPtr _thread;
};

}

#endif
