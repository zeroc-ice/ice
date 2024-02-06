//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include "Config.h"
#include <memory>

namespace Ice
{

class ObjectPrx;
/// \cond INTERNAL
using ObjectPrxPtr = ::std::shared_ptr<ObjectPrx>;
/// \endcond

}

#endif
