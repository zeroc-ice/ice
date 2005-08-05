// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_ROUTING_TABLE_F_H
#define ICEE_ROUTING_TABLE_F_H

#include <IceE/Handle.h>

namespace IceInternal
{

class RoutingTable;
ICEE_API void incRef(RoutingTable*);
ICEE_API void decRef(RoutingTable*);
typedef IceInternal::Handle<RoutingTable> RoutingTablePtr;

}

#endif
