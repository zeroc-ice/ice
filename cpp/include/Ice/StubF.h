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

namespace _IceIntf { namespace Ice { class Object; } }
namespace _IceStub { namespace Ice { class Object; } }
namespace _IceStubM { namespace Ice { class Object; } }

namespace _Ice
{

void ICE_API _incRef(::_IceIntf::Ice::Object*);
void ICE_API _decRef(::_IceIntf::Ice::Object*);

void ICE_API _incRef(::_IceStub::Ice::Object*);
void ICE_API _decRef(::_IceStub::Ice::Object*);

void ICE_API _incRef(::_IceStubM::Ice::Object*);
void ICE_API _decRef(::_IceStubM::Ice::Object*);

}

namespace Ice
{

typedef _Ice::Handle< ::_IceIntf::Ice::Object > Object;

}

#endif
