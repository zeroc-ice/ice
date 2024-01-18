//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCATOR_F_H
#define ICE_LOCATOR_F_H

// TODO: remove this include when we remove the C++98 mapping.
#include <IceUtil/PushDisableWarnings.h>

#include "Config.h"
#include <memory>
#include "ProxyF.h"
#include "SharedPtr.h"

#ifdef ICE_CPP11_MAPPING // C++11 mapping

namespace Ice
{

class Locator;
class LocatorPrx;
class LocatorRegistry;
class LocatorRegistryPrx;

using LocatorPtr = ::std::shared_ptr<Locator>;
using LocatorPrxPtr = ::std::shared_ptr<LocatorPrx>;

using LocatorRegistryPtr = ::std::shared_ptr<LocatorRegistry>;
using LocatorRegistryPrxPtr = ::std::shared_ptr<LocatorRegistryPrx>;

}

#else // C++98 mapping

namespace Ice
{
    class InputStream;
}

namespace IceProxy::Ice
{

class Locator;
/// \cond INTERNAL
ICE_API void _readProxy(::Ice::InputStream*, ::IceInternal::ProxyHandle< Locator>&);
ICE_API ::IceProxy::Ice::Object* upCast(Locator*);
/// \endcond

class LocatorRegistry;
/// \cond INTERNAL
ICE_API void _readProxy(::Ice::InputStream*, ::IceInternal::ProxyHandle< LocatorRegistry>&);
ICE_API ::IceProxy::Ice::Object* upCast(LocatorRegistry*);
/// \endcond

}

namespace Ice
{

typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::Locator> LocatorPrx;
typedef LocatorPrx LocatorPrxPtr;

class Locator;
using LocatorPtr = ::Ice::SharedPtr< Locator>;
typedef ::IceInternal::ProxyHandle< ::IceProxy::Ice::LocatorRegistry> LocatorRegistryPrx;
typedef LocatorRegistryPrx LocatorRegistryPrxPtr;

class LocatorRegistry;
using LocatorRegistryPtr = ::Ice::SharedPtr< LocatorRegistry>;

}

#endif

// TODO: remove this include when we remove the C++98 mapping.
#include <IceUtil/PopDisableWarnings.h>
#endif
