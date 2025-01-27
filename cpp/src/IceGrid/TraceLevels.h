// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_TRACE_LEVELS_H
#define ICEGRID_TRACE_LEVELS_H

#include "Ice/CommunicatorF.h"
#include "Ice/Logger.h"

namespace IceGrid
{
    class TraceLevels // NOLINT(clang-analyzer-optin.performance.Padding)
    {
    public:
        TraceLevels(const Ice::CommunicatorPtr&, const std::string&);

        const int admin{0};
        const std::string adminCat;

        const int application{0};
        const std::string applicationCat;

        const int node{0};
        const std::string nodeCat;

        const int replica{0};
        const std::string replicaCat;

        const int server{0};
        const std::string serverCat;

        const int adapter{0};
        const std::string adapterCat;

        const int object{0};
        const std::string objectCat;

        const int activator{0};
        const std::string activatorCat;

        const int locator{0};
        const std::string locatorCat;

        const int session{0};
        const std::string sessionCat;

        const int discovery{0};
        const std::string discoveryCat;

        const Ice::LoggerPtr logger;
    };

} // End namespace IceGrid

#endif
