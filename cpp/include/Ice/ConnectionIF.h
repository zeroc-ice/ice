//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTIONI_F_H
#define ICE_CONNECTIONI_F_H

#include <Ice/Handle.h>
#include <Ice/LocalObject.h>

namespace Ice
{

/// \cond INTERNAL
class ConnectionI;
using ConnectionIPtr = ::std::shared_ptr<ConnectionI>;
/// \endcond

}

namespace IceInternal
{

enum AsyncStatus
{
    AsyncStatusQueued = 0,
    AsyncStatusSent = 1,
    AsyncStatusInvokeSentCallback = 2
};

}

#endif
