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

namespace __Ice
{

class AcceptorI;
void incRef(AcceptorI*);
void decRef(AcceptorI*);
typedef __Ice::Handle<AcceptorI> Acceptor;

}

#endif
