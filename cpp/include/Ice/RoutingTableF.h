// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ROUTING_TABLE_F_H
#define ICE_ROUTING_TABLE_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class RoutingTable;
ICE_API void incRef(RoutingTable*);
ICE_API void decRef(RoutingTable*);
typedef IceInternal::Handle<RoutingTable> RoutingTablePtr;

}

#endif
