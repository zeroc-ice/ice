// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_ROUTING_TABLE_H
#define ICE_ROUTING_TABLE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/RoutingTableF.h>
#include <Ice/ProxyF.h>
#include <Ice/Identity.h>

namespace Ice
{

struct Identity;

}

namespace IceInternal
{

class RoutingTable : public IceUtil::Shared, public IceUtil::Mutex
{
public:

    RoutingTable();

    //
    // Clear the contents of the routing table.
    //
    void clear();

    //
    // Returns false if the Proxy exists already.
    //
    bool add(const Ice::ObjectPrx&);

private:

    std::map<Ice::Identity, Ice::ObjectPrx> _table;
    std::map<Ice::Identity, Ice::ObjectPrx>::iterator _tableHint;
};

}

#endif
