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

namespace _Ice
{

void ICE_API _incRef(::Ice::CommunicatorI*);
void ICE_API _decRef(::Ice::CommunicatorI*);

}

namespace Ice
{

typedef _Ice::Handle<CommunicatorI> Communicator;

}

#endif
