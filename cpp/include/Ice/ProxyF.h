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

#include <IceUtil/Shared.h>

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

inline ::IceProxy::Ice::Object* 
upCast(::IceProxy::Ice::Object* o)
{
    return o;
}

inline ::IceDelegate::Ice::Object*
upCast(::IceDelegate::Ice::Object* o)
{
    return o;
}

inline ::IceDelegateD::Ice::Object*
upCast(::IceDelegateD::Ice::Object* o)
{
    return o;
}

inline ::IceDelegateM::Ice::Object*
upCast(::IceDelegateM::Ice::Object* o)
{
    return o;
}

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}


#endif
