// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_STUB_F_H
#define ICE_STUB_F_H

#include <Ice/Handle.h>

namespace Ice { class ObjectI; }
namespace __IceStub { namespace Ice { class ObjectI; } }
namespace __IceStubM { namespace Ice { class ObjectI; } }

namespace __Ice
{

void ICE_API incRef(::Ice::ObjectI*);
void ICE_API decRef(::Ice::ObjectI*);

void ICE_API incRef(::__IceStub::Ice::ObjectI*);
void ICE_API decRef(::__IceStub::Ice::ObjectI*);

void ICE_API incRef(::__IceStubM::Ice::ObjectI*);
void ICE_API decRef(::__IceStubM::Ice::ObjectI*);

}

namespace Ice
{

typedef __Ice::Handle< ObjectI > Object;

}

namespace __IceStub { namespace Ice
{

typedef __Ice::Handle< ObjectI > Object;

} }

namespace __IceStubM { namespace Ice
{

typedef __Ice::Handle< ObjectI > Object;

} }

#endif
