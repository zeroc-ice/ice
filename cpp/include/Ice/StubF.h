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

namespace __IceIntf { namespace Ice { class Object; } }
namespace __IceStub { namespace Ice { class Object; } }
namespace __IceStubM { namespace Ice { class Object; } }

namespace __Ice
{

void ICE_API incRef(::__IceIntf::Ice::Object*);
void ICE_API decRef(::__IceIntf::Ice::Object*);

void ICE_API incRef(::__IceStub::Ice::Object*);
void ICE_API decRef(::__IceStub::Ice::Object*);

void ICE_API incRef(::__IceStubM::Ice::Object*);
void ICE_API decRef(::__IceStubM::Ice::Object*);

}

namespace Ice
{

typedef __Ice::Handle< ::__IceIntf::Ice::Object > Object;

}

#endif
