// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_ACCEPTOR_F_H
#define ICE_ACCEPTOR_F_H

#include <Ice/Handle.h>

namespace IceInternal
{

class Acceptor;
ICE_PROTOCOL_API void incRef(Acceptor*);
ICE_PROTOCOL_API void decRef(Acceptor*);
typedef Handle<Acceptor> AcceptorPtr;

}

#endif
