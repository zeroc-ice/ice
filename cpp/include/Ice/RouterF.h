//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ROUTER_F_H
#define ICE_ROUTER_F_H

#include "Config.h"
#include <memory>
#include "ProxyF.h"
#include "SharedPtr.h"

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace Ice
{

class Router;
class RouterPrx;

using RouterPtr = ::std::shared_ptr<Router>;
using RouterPrxPtr = ::std::shared_ptr<RouterPrx>;

}

#else // C++98 mapping

namespace Ice
{
    class InputStream;
}

namespace IceProxy::Ice
{

class Router;
/// \cond INTERNAL
ICE_API void _readProxy(::Ice::InputStream*, ::IceInternal::ProxyHandle< Router>&);
ICE_API ::IceProxy::Ice::Object* upCast(Router*);
/// \endcond

}

namespace Ice
{
typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Router> RouterPrx;
typedef RouterPrx RouterPrxPtr;

class Router;
using RouterPtr = ::Ice::SharedPtr< Router>;

}

#endif

#endif
