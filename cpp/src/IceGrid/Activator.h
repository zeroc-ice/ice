//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_ACTIVATOR_H
#define ICE_GRID_ACTIVATOR_H

#include <IceGrid/Internal.h>

#ifndef _WIN32
#   include <sys/types.h> // for uid_t, gid_t
#endif

namespace IceGrid
{

class TraceLevels;
class ServerI;

std::string signalToString(int);

class Activator final : public std::enable_shared_from_this<Activator>
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
        std::shared_ptr<ServerI> server;
    };

    Activator(const std::shared_ptr<TraceLevels>&);
    ~Activator();

    int activate(const std::string&, const std::string&, const std::string&,
#ifndef _WIN32
                 uid_t, gid_t,
#endif
                 const Ice::StringSeq&, const Ice::StringSeq&, const std::shared_ptr<ServerI>&);
    void deactivate(const std::string&, const std::shared_ptr<Ice::ProcessPrx>&);
    void kill(const std::string&);
    void sendSignal(const std::string&, const std::string&);

    int getServerPid(const std::string&);

    void start();
    void waitForShutdown();
    void shutdown();
    void destroy();

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

    std::shared_ptr<TraceLevels> _traceLevels;
    std::map<std::string, Process> _processes;
    bool _deactivating;

#ifdef _WIN32
    HANDLE _hIntr;
    std::vector<Process*> _terminated;
#else
    int _fdIntrRead;
    int _fdIntrWrite;
#endif

    std::mutex _mutex;
    std::condition_variable _condVar;
    std::thread _thread;
};

}

#endif
