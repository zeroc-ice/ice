// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include <Ice/ProxyHandle.h>

namespace __IceProxy { namespace Ice { class Object; } }
namespace __IceDelegate { namespace Ice { class Object; } }
namespace __IceDelegateM { namespace Ice { class Object; } }

namespace __Ice
{

void ICE_API incRef(::__IceProxy::Ice::Object*);
void ICE_API decRef(::__IceProxy::Ice::Object*);

void ICE_API incRef(::__IceDelegate::Ice::Object*);
void ICE_API decRef(::__IceDelegate::Ice::Object*);

void ICE_API incRef(::__IceDelegateM::Ice::Object*);
void ICE_API decRef(::__IceDelegateM::Ice::Object*);

void ICE_API checkedCast(::__IceProxy::Ice::Object*,
			 ::__IceProxy::Ice::Object*&);
void ICE_API uncheckedCast(::__IceProxy::Ice::Object*,
			   ::__IceProxy::Ice::Object*&);

}

namespace Ice
{

typedef __Ice::ProxyHandle< ::__IceProxy::Ice::Object> Object_prx;

}

#endif
