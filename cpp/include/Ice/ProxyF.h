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

namespace IceProxy { namespace Ice { class Object; } }
namespace IceDelegate { namespace Ice { class Object; } }
namespace IceDelegateM { namespace Ice { class Object; } }
namespace IceDelegateD { namespace Ice { class Object; } }

namespace IceInternal
{

void ICE_API incRef(::IceProxy::Ice::Object*);
void ICE_API decRef(::IceProxy::Ice::Object*);

void ICE_API incRef(::IceDelegate::Ice::Object*);
void ICE_API decRef(::IceDelegate::Ice::Object*);

void ICE_API incRef(::IceDelegateM::Ice::Object*);
void ICE_API decRef(::IceDelegateM::Ice::Object*);

void ICE_API incRef(::IceDelegateD::Ice::Object*);
void ICE_API decRef(::IceDelegateD::Ice::Object*);

void ICE_API checkedCast(::IceProxy::Ice::Object*,
			 ::IceProxy::Ice::Object*&);
void ICE_API uncheckedCast(::IceProxy::Ice::Object*,
			   ::IceProxy::Ice::Object*&);

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

#endif
