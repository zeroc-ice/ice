// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ACTIVATOR_H
#define ICE_GRID_ACTIVATOR_H

#include <IceUtil/Thread.h>
#include <IceGrid/Internal.h>

#ifndef _WIN32
#   include <sys/types.h> // for uid_t, gid_t
#endif

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ServerI;
typedef IceUtil::Handle<ServerI> ServerIPtr;

std::string signalToString(int);

class Activator : public IceUtil::Monitor< IceUtil::Mutex>, public IceUtil::Shared
{
public:

    struct Process
    {
#ifdef _WIN32
        Activator* activator;
        DWORD pid;
        HANDLE hnd;
        HANDLE waithnd;
#else
        pid_t pid;
        int pipeFd;
        std::string msg;
#endif
        ServerIPtr server;
    };

    Activator(const TraceLevelsPtr&);
    virtual ~Activator();

    virtual int activate(const std::string&, const std::string&, const std::string&,
#ifndef _WIN32
                         uid_t, gid_t, 
#endif
                         const Ice::StringSeq&, const Ice::StringSeq&, const ServerIPtr&);
    virtual void deactivate(const std::string&, const Ice::ProcessPrx&);
    virtual void kill(const std::string&);
    virtual void sendSignal(const std::string&, const std::string&);
  
    virtual Ice::Int getServerPid(const std::string&);
    
    virtual void start();
    virtual void waitForShutdown();
    virtual void shutdown();
    virtual void destroy();
    
    bool isActive();
    
    void sendSignal(const std::string&, int);
    void runTerminationListener();

#ifdef _WIN32
    void processTerminated(Process*);
#endif

private:

    void terminationListener();
    void clearInterrupt();
    void setInterrupt();
    
#ifndef _WIN32
    int waitPid(pid_t);
#endif

    TraceLevelsPtr _traceLevels;
    std::map<std::string, Process> _processes;
    bool _deactivating;

#ifdef _WIN32
    HANDLE _hIntr;
    std::vector<Process*> _terminated;
#else
    int _fdIntrRead;
    int _fdIntrWrite;
#endif

    IceUtil::ThreadPtr _thread;
};
typedef IceUtil::Handle<Activator> ActivatorPtr;

}

#endif
