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
