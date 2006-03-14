// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_CONNECTOR_F_H
#define ICE_CONNECTOR_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Connector;
ICE_API void incRef(Connector*);
ICE_API void decRef(Connector*);
typedef Handle<Connector> ConnectorPtr;

}

#endif
