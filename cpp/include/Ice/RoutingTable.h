// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ROUTING_TABLE_H
#define ICE_ROUTING_TABLE_H

#include <IceUtil/Shared.h>
#include <IceUtil/Mutex.h>
#include <Ice/RoutingTableF.h>
#include <Ice/ProxyF.h>

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
