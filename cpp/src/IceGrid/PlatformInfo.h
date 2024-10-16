//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_PLATFORM_INFO_H
#define ICEGRID_PLATFORM_INFO_H

#include "Internal.h"

#ifdef _WIN32
#    include <deque>
#    include <pdh.h> // Performance data helper API
#endif

namespace IceGrid
{
    class TraceLevels;

    NodeInfo toNodeInfo(const std::shared_ptr<InternalNodeInfo>&);
    RegistryInfo toRegistryInfo(const std::shared_ptr<InternalReplicaInfo>&);

    class PlatformInfo final
    {
    public:
        PlatformInfo(const std::string&, const Ice::CommunicatorPtr&, const std::shared_ptr<TraceLevels>&);

        void start();
        void stop();

        std::shared_ptr<InternalNodeInfo> getInternalNodeInfo() const;
        std::shared_ptr<InternalReplicaInfo> getInternalReplicaInfo() const;

        NodeInfo getNodeInfo() const;
        RegistryInfo getRegistryInfo() const;

        LoadInfo getLoadInfo() const;
        int getProcessorSocketCount() const;
        std::string getHostname() const;
        std::string getDataDir() const;
        std::string getCwd() const;

#if defined(_WIN32)
        void runUpdateLoadInfo();
#endif

    private:
        const std::shared_ptr<TraceLevels> _traceLevels;
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
        std::deque<int> _usages1;
        std::deque<int> _usages5;
        std::deque<int> _usages15;
        int _last1Total;
        int _last5Total;
        int _last15Total;
        bool _terminated;
        mutable std::mutex _utilizationMutex;
        std::condition_variable _utilizationCondVar;
        std::thread _updateUtilizationThread;
#endif
    };

};

#endif
