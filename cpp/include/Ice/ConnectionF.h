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
void incRef(Connection*);
void decRef(Connection*);
typedef IceInternal::Handle<Connection> ConnectionPtr;

}

#endif
