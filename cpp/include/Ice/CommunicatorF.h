// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

// Generated from file `CommunicatorF.ice'

#ifndef __CommunicatorF_h__
#define __CommunicatorF_h__

#include <Ice/LocalObject.h>

namespace Ice
{

class Communicator;

}

namespace __Ice
{

void ICE_API incRef(::Ice::Communicator*);
void ICE_API decRef(::Ice::Communicator*);

}

namespace Ice
{

typedef ::__Ice::Handle< ::Ice::Communicator> Communicator_ptr;

}

#endif
