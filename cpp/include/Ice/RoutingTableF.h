// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
