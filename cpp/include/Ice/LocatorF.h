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
using LocatorPtr = ::std::shared_ptr<Locator>;

class LocatorPrx;

class LocatorRegistry;
using LocatorRegistryPtr = ::std::shared_ptr<LocatorRegistry>;

class LocatorRegistryPrx;

}

#endif
