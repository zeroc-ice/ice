// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
