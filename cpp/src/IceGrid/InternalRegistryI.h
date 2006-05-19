// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_INTERNALREGISTRYI_H
#define ICE_GRID_INTERNALREGISTRYI_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;    

class InternalRegistryI : public InternalRegistry
{
public:

    InternalRegistryI(const DatabasePtr&, const ReapThreadPtr&, const NodeObserverPrx&, int);
    virtual ~InternalRegistryI();

    virtual NodeSessionPrx registerNode(const std::string&, const NodePrx&, const NodeInfo&, const Ice::Current&);

private:    

    const DatabasePtr _database;
    const ReapThreadPtr _nodeReaper;
    const NodeObserverPrx _nodeObserver;
    const int _nodeSessionTimeout;
};
    
};

#endif
