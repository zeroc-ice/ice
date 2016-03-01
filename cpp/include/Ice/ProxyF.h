// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
inline Object* upCast(Object* o) { return o; }

}

}

namespace IceDelegate
{

namespace Ice
{

class Object;
inline Object* upCast(Object* o) { return o; }

}

}

namespace IceDelegateM
{

namespace Ice
{

class Object;
inline Object* upCast(Object* o) { return o; }

}

}

namespace IceDelegateD
{

namespace Ice
{

class Object;
inline Object* upCast(Object* o) { return o; }

}

}

namespace Ice
{

typedef IceInternal::ProxyHandle< ::IceProxy::Ice::Object> ObjectPrx;

}


#endif
