// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include <Ice/Config.h>
#include <Ice/ProxyHandle.h>

#ifndef ICE_CPP11_MAPPING // C++98 mapping
namespace IceProxy
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
typedef ObjectPrx ObjectPrxPtr;

}

#else // C++11 mapping

namespace Ice
{

class ObjectPrx;
typedef ::std::shared_ptr<ObjectPrx> ObjectPrxPtr;

}

namespace IceInternal
{

template<typename P>
::std::shared_ptr<P> createProxy();

}

#endif

#endif
