//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_PROXY_F_H
#define ICE_PROXY_F_H

#include <Ice/Config.h>
#include <Ice/ProxyHandle.h>

namespace Ice
{

class ObjectPrx;
/// \cond INTERNAL
using ObjectPrxPtr = ::std::shared_ptr<ObjectPrx>;
/// \endcond

}

namespace IceInternal
{

template<typename P>
::std::shared_ptr<P> createProxy();

}

#endif
