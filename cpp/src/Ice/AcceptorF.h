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
void incRef(Acceptor*);
void decRef(Acceptor*);
typedef Handle<Acceptor> AcceptorPtr;

}

#endif
