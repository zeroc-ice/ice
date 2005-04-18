// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_NODE_I_H
#define ICE_GRID_NODE_I_H

#include <IceGrid/Internal.h>

namespace IceGrid
{

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class Activator;
typedef IceUtil::Handle<Activator> ActivatorPtr;

class WaitQueue;
typedef IceUtil::Handle<WaitQueue> WaitQueuePtr;

class NodeI : public Node, public IceUtil::Mutex
{
public:

    NodeI(const Ice::CommunicatorPtr&, const ActivatorPtr&, const WaitQueuePtr&, const TraceLevelsPtr&, 
	  const std::string&);

    virtual ServerPrx loadServer(const ServerDescriptorPtr&, StringAdapterPrxDict&, const Ice::Current&);
    virtual void destroyServer(const ServerDescriptorPtr&, const Ice::Current&);
    
    virtual std::string getName(const Ice::Current&) const;
    virtual std::string getHostname(const Ice::Current&) const;
    virtual void shutdown(const Ice::Current&) const;

    WaitQueuePtr getWaitQueue() const;
    Ice::CommunicatorPtr getCommunicator() const;
    ActivatorPtr getActivator() const;
    TraceLevelsPtr getTraceLevels() const;

    void checkConsistency(const Ice::StringSeq&);
    bool canRemoveServerDirectory(const std::string&);
    void removeServerDirectory(const std::string&, const std::string&);

private:

    const Ice::CommunicatorPtr _communicator;
    const ActivatorPtr _activator;
    const WaitQueuePtr _waitQueue;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const std::string _hostname;
    std::string _serversDir;
    std::string _tmpDir;
};
typedef IceUtil::Handle<NodeI> NodeIPtr;

}

#endif
