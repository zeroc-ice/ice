// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTING_TABLE_H
#define ICEE_ROUTING_TABLE_H

#include <IceE/RoutingTableF.h>
#include <IceE/ProxyF.h>

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/Identity.h>

namespace Ice
{

struct Identity;

}

namespace IceInternal
{

class ICEE_API RoutingTable : public IceUtil::Shared, public IceUtil::Mutex
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

    //
    // Returns null if no Proxy exists for the given identity.
    //
    Ice::ObjectPrx get(const Ice::Identity&);

private:

    std::map<Ice::Identity, Ice::ObjectPrx> _table;
    std::map<Ice::Identity, Ice::ObjectPrx>::iterator _tableHint;
};

}

#endif
