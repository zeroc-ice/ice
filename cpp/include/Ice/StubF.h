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

namespace _IceObj { namespace Ice { class ObjectI; } }
namespace _IceStub { namespace Ice { class ObjectI; } }
namespace _IceStubM { namespace Ice { class ObjectI; } }

namespace _Ice
{

void ICE_API _incRef(::_IceObj::Ice::ObjectI*);
void ICE_API _decRef(::_IceObj::Ice::ObjectI*);

void ICE_API _incRef(::_IceStub::Ice::ObjectI*);
void ICE_API _decRef(::_IceStub::Ice::ObjectI*);

void ICE_API _incRef(::_IceStubM::Ice::ObjectI*);
void ICE_API _decRef(::_IceStubM::Ice::ObjectI*);

}

namespace Ice
{

typedef _Ice::Handle< ::_IceObj::Ice::ObjectI > Object;

}

namespace _IceStub { namespace Ice
{

typedef _Ice::Handle< ::_IceStub::Ice::ObjectI > Object;

} }

namespace _IceStubM { namespace Ice
{

typedef _Ice::Handle< ::_IceStubM::Ice::ObjectI > Object;

} }

#endif
