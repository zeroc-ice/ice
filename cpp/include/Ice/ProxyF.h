// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include <Ice/ProxyHandle.h>

namespace IceProxy
{

namespace Ice
{

class Object;

}

}

namespace IceDelegate
{

namespace Ice
{

class Object;

}

}

namespace IceDelegateM
{

namespace Ice
{

class Object;

}

}

namespace IceDelegateD
{

namespace Ice
{

class Object;

}

}

namespace IceInternal
{

ICE_API void incRef(::IceProxy::Ice::Object*);
ICE_API void decRef(::IceProxy::Ice::Object*);

ICE_API void incRef(::IceDelegate::Ice::Object*);
ICE_API void decRef(::IceDelegate::Ice::Object*);

ICE_API void incRef(::IceDelegateM::Ice::Object*);
ICE_API void decRef(::IceDelegateM::Ice::Object*);

ICE_API void incRef(::IceDelegateD::Ice::Object*);
ICE_API void decRef(::IceDelegateD::Ice::Object*);

ICE_API void checkedCast(const ProxyHandle< ::IceProxy::Ice::Object>&,
			 ProxyHandle< ::IceProxy::Ice::Object>&);
ICE_API void checkedCast(const ProxyHandle< ::IceProxy::Ice::Object>&, const ::std::string&,
			 ProxyHandle< ::IceProxy::Ice::Object>&);
ICE_API void uncheckedCast(const ProxyHandle< ::IceProxy::Ice::Object>&,
			   ProxyHandle< ::IceProxy::Ice::Object>&);
ICE_API void uncheckedCast(const ProxyHandle< ::IceProxy::Ice::Object>&, const ::std::string&,
			   ProxyHandle< ::IceProxy::Ice::Object>&);

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

#endif
