// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_SSL_CONNECTION_F_H
#define ICE_SSL_CONNECTION_F_H

#include <Ice/Handle.h>

namespace IceSSL
{

class Connection;
typedef IceInternal::Handle<Connection> ConnectionPtr;

}

namespace IceInternal
{

void incRef(::IceSSL::Connection*);
void decRef(::IceSSL::Connection*);

}

#endif
