// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTING_TABLE_H
#define ICEE_ROUTING_TABLE_H

#include <IceE/Shared.h>
#include <IceE/Mutex.h>
#include <IceE/RoutingTableF.h>
#include <IceE/ProxyF.h>
#include <IceE/Identity.h>

namespace IceE
{

struct Identity;

}

namespace IceEInternal
{

class ICEE_API RoutingTable : public IceE::Shared, public IceE::Mutex
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
    bool add(const IceE::ObjectPrx&);

    //
    // Returns null if no Proxy exists for the given identity.
    //
    IceE::ObjectPrx get(const IceE::Identity&);

private:

    std::map<IceE::Identity, IceE::ObjectPrx> _table;
    std::map<IceE::Identity, IceE::ObjectPrx>::iterator _tableHint;
};

}

#endif
