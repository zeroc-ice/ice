// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_COMMUNICATOR_F_H
#define ICE_COMMUNICATOR_F_H

#include <Ice/Handle.h>

namespace Ice { class CommunicatorI; }

namespace __Ice
{

void ICE_API incRef(::Ice::CommunicatorI*);
void ICE_API decRef(::Ice::CommunicatorI*);

}

namespace Ice
{

typedef __Ice::Handle<CommunicatorI> Communicator;

}

#endif
