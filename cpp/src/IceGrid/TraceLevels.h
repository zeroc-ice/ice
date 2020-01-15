//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_TRACE_LEVELS_H
#define ICE_GRID_TRACE_LEVELS_H

#include <Ice/CommunicatorF.h>
#include <Ice/LoggerF.h>

namespace IceGrid
{

class TraceLevels
{
public:

    TraceLevels(const std::shared_ptr<Ice::Communicator>&, const std::string&);

    const int admin;
    const std::string adminCat;

    const int application;
    const std::string applicationCat;

    const int node;
    const std::string nodeCat;

    const int replica;
    const std::string replicaCat;

    const int server;
    const std::string serverCat;

    const int adapter;
    const std::string adapterCat;

    const int object;
    const std::string objectCat;

    const int activator;
    const std::string activatorCat;

    const int locator;
    const std::string locatorCat;

    const int session;
    const std::string sessionCat;

    const int discovery;
    const std::string discoveryCat;

    const std::shared_ptr<Ice::Logger> logger;
};

} // End namespace IceGrid

#endif
