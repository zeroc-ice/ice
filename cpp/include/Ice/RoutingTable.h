// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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

};

namespace IceInternal
{

class ICE_API RoutingTable : public ::IceUtil::Shared, public ::IceUtil::Mutex
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
    bool add(const ::Ice::ObjectPrx&);

    //
    // Returns null if no Proxy exists for the given identity.
    //
    ::Ice::ObjectPrx get(const ::Ice::Identity&);

private:

    std::map< ::Ice::Identity, ::Ice::ObjectPrx> _table;
    std::map< ::Ice::Identity, ::Ice::ObjectPrx>::iterator _tableHint;
};

}

#endif
