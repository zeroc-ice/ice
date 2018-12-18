// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_TRACE_LEVELS_H
#define ICE_GRID_TRACE_LEVELS_H

#include <IceUtil/Shared.h>
#include <Ice/CommunicatorF.h>
#include <Ice/LoggerF.h>

namespace IceGrid
{

class TraceLevels : public ::IceUtil::Shared
{
public:

    TraceLevels(const Ice::CommunicatorPtr&, const std::string&);
    virtual ~TraceLevels();

    const int admin;
    const char* adminCat;

    const int application;
    const char* applicationCat;

    const int node;
    const char* nodeCat;

    const int replica;
    const char* replicaCat;

    const int server;
    const char* serverCat;

    const int adapter;
    const char* adapterCat;

    const int object;
    const char* objectCat;

    const int activator;
    const char* activatorCat;

    const int patch;
    const char* patchCat;

    const int locator;
    const char* locatorCat;

    const int session;
    const char* sessionCat;

    const int discovery;
    const char* discoveryCat;

    const Ice::LoggerPtr logger;
};

typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

} // End namespace IceGrid

#endif
