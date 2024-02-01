//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCATOR_F_H
#define ICE_LOCATOR_F_H

#include "Config.h"
#include <memory>
#include "ProxyF.h"

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

#endif
