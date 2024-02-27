//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_CONNECTIONI_F_H
#define ICE_CONNECTIONI_F_H

#include <memory>

namespace Ice
{

class ConnectionI;
using ConnectionIPtr = ::std::shared_ptr<ConnectionI>;

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
