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
#include <IceGrid/Descriptor.ice>

module IceGrid
{

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
    void init(int serial, ApplicationDescriptorSeq applications, Ice::StringSeq nodesUp);

    void applicationAdded(int serial, ApplicationDescriptor desc);
    void applicationRemoved(int serial, string name);
    void applicationSynced(int serial, ApplicationDescriptor desc);
    void applicationUpdated(int serial, ApplicationUpdateDescriptor desc);
    
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
