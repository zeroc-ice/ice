// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_CONNECTION_F_H
#define ICE_CONNECTION_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Connection;
ICE_API void incRef(Connection*);
ICE_API void decRef(Connection*);
typedef IceInternal::Handle<Connection> ConnectionPtr;

}

#endif
