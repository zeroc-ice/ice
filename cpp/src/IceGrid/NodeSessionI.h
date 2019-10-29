//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEGRID_NODE_SESSION_H
#define ICEGRID_NODE_SESSION_H

#include <IceGrid/Internal.h>
#include <set>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class NodeSessionI : public NodeSession, public IceUtil::Mutex
{
public:

    NodeSessionI(const DatabasePtr&, const NodePrx&, const InternalNodeInfoPtr&, int, const LoadInfo&);

    virtual void keepAlive(const LoadInfo&, const Ice::Current&);
    virtual void setReplicaObserver(const ReplicaObserverPrx&, const Ice::Current&);
    virtual int getTimeout(const Ice::Current&) const;
    virtual NodeObserverPrx getObserver(const Ice::Current&) const;
    virtual void loadServers_async(const AMD_NodeSession_loadServersPtr&, const Ice::Current&) const;
    virtual Ice::StringSeq getServers(const Ice::Current&) const;
    virtual void waitForApplicationUpdate_async(const AMD_NodeSession_waitForApplicationUpdatePtr&,
                                                const std::string&, int, const Ice::Current&) const;
    virtual void destroy(const Ice::Current&);

    virtual IceUtil::Time timestamp() const;
    virtual void shutdown();

    const NodePrx& getNode() const;
    const InternalNodeInfoPtr& getInfo() const;
    const LoadInfo& getLoadInfo() const;
    NodeSessionPrx getProxy() const;

    bool isDestroyed() const;

private:

    void destroyImpl(bool);

    const DatabasePtr _database;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const NodePrx _node;
    const InternalNodeInfoPtr _info;
    const int _timeout;
    NodeSessionPrx _proxy;
    ReplicaObserverPrx _replicaObserver;
    IceUtil::Time _timestamp;
    LoadInfo _load;
    bool _destroy;
};
typedef IceUtil::Handle<NodeSessionI> NodeSessionIPtr;

};

#endif
