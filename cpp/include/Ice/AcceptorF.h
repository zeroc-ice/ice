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

namespace _Ice
{

class AcceptorI;
void ICE_API _incRef(AcceptorI*);
void ICE_API _decRef(AcceptorI*);
typedef _Ice::Handle<AcceptorI> Acceptor;

}

#endif
