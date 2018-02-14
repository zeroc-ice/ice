// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_PLATFORM_INFO_H
#define ICE_GRID_PLATFORM_INFO_H

#include <IceUtil/Thread.h>
#include <IceGrid/Internal.h>

#ifdef _WIN32
#   include <pdh.h> // Performance data helper API
#   include <deque>
#endif



namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

NodeInfo toNodeInfo(const InternalNodeInfoPtr&);
RegistryInfo toRegistryInfo(const InternalReplicaInfoPtr&);

class PlatformInfo
{
public:

    PlatformInfo(const std::string&, const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    ~PlatformInfo();

    void start();
    void stop();

    InternalNodeInfoPtr getInternalNodeInfo() const;
    InternalReplicaInfoPtr getInternalReplicaInfo() const;

    NodeInfo getNodeInfo() const;
    RegistryInfo getRegistryInfo() const;

    LoadInfo getLoadInfo();
    int getProcessorSocketCount() const;
    std::string getHostname() const;
    std::string getDataDir() const;
    std::string getCwd() const;

#if defined(_WIN32)
    void runUpdateLoadInfo();
#endif

private:

    const TraceLevelsPtr _traceLevels;
    std::string _name;
    std::string _os;
    std::string _hostname;
    std::string _release;
    std::string _version;
    std::string _machine;
    int _nProcessorThreads;
    std::string _dataDir;
    std::string _cwd;
    std::string _endpoints;
    int _nProcessorSockets;

#if defined(_WIN32)
    IceUtil::ThreadPtr _updateUtilizationThread;
    IceUtil::Monitor<IceUtil::Mutex> _utilizationMonitor;
    bool _terminated;
    std::deque<int> _usages1;
    std::deque<int> _usages5;
    std::deque<int> _usages15;
    int _last1Total;
    int _last5Total;
    int _last15Total;
#elif defined(_AIX)
    int _kmem;
#endif

};


};

#endif
