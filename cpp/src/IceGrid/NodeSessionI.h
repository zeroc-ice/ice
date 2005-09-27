// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_NODE_SESSION_H
#define ICEGRID_NODE_SESSION_H

#include <IceGrid/Internal.h>
#include <IceGrid/SessionI.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class NodeSessionI : public NodeSession, public SessionI,  public IceUtil::Mutex
{
public:

    NodeSessionI(const DatabasePtr&, const std::string&, const NodePrx&, const NodeInfo&);

    virtual void keepAlive(const LoadInfo&, const Ice::Current&);
    virtual Ice::StringSeq getServers(const Ice::Current&);
    virtual void destroy(const Ice::Current&);
    
    const NodePrx& getNode() const;
    const NodeInfo& getInfo() const;
    const LoadInfo& getLoadInfo() const;
    virtual IceUtil::Time timestamp() const;

private:
    
    const DatabasePtr _database;
    const TraceLevelsPtr _traceLevels;
    const std::string _name;
    const NodePrx _node;
    const NodeInfo _info;
    const IceUtil::Time _startTime;
    IceUtil::Time _timestamp;
    LoadInfo _load;
    bool _destroy;
};
typedef IceUtil::Handle<NodeSessionI> NodeSessionIPtr;

};

#endif
