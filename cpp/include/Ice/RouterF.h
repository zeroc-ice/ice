//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ROUTER_F_H
#define ICE_ROUTER_F_H

#include "Config.h"
#include <memory>
#include "ProxyF.h"

namespace Ice
{

class Router;
class RouterPrx;

using RouterPtr = ::std::shared_ptr<Router>;
using RouterPrxPtr = ::std::shared_ptr<RouterPrx>;

}

#endif
