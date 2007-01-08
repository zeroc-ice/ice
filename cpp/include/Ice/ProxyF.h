// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include <Ice/Config.h>
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

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}

#endif
