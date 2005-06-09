// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_OBSERVER_ICE
#define ICE_GRID_OBSERVER_ICE

#include <Glacier2/Session.ice>

module IceGrid
{

/**
 *
 * An enumeration representing the state of the server.
 *
 **/
enum ServerState
{
    /**
     *
     * The server is not running.
     *
     **/
    Inactive,

    /**
     *
     * The server is being activated and will change to the active
     * state if the server fork succeeded or to the Inactive state if
     * it failed.
     *
     **/
    Activating,

    /**
     *
     * The server is running.
     *
     **/
    Active,
    
    /**
     *
     * The server is being deactivated.
     *
     **/
    Deactivating,

    /**
     *
     * The server is being destroyed.
     *
     **/
    Destroying,

    /**
     *
     * The server is destroyed.
     *
     **/
    Destroyed
};

struct ServerDynamicInfo
{
    string name;
    ServerState state;
    int pid;
};
sequence<ServerDynamicInfo> ServerDynamicInfoSeq;

struct AdapterDynamicInfo
{
    string id;
    Object* proxy;
};
sequence<AdapterDynamicInfo> AdapterDynamicInfoSeq;

interface NodeObserver
{
    void init(string node, ServerDynamicInfoSeq servers, AdapterDynamicInfoSeq adapters);

    void updateServer(string node, ServerDynamicInfo updatedInfo);

    void updateAdapter(string node, AdapterDynamicInfo updatedInfo);
};

interface RegistryObserver
{
    void nodeUp(string name);
    void nodeDown(string name);
};

interface Session extends Glacier2::Session
{
    void keepAlive();
    void setObservers(RegistryObserver* registryObs, NodeObserver* nodeObs);
};

interface SessionManager extends Glacier2::SessionManager
{
    Session* createLocalSession(string userId);
};

};

#endif
