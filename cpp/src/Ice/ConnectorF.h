// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_CONNECTOR_F_H
#define ICE_CONNECTOR_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Connector;
ICE_PROTOCOL_API void incRef(Connector*);
ICE_PROTOCOL_API void decRef(Connector*);
typedef Handle<Connector> ConnectorPtr;

}

#endif
