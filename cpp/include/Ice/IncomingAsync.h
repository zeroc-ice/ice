//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include "IncomingAsyncF.h"
#include "Incoming.h"

namespace IceInternal
{

// Represents an incoming request dispatched with AMD.
class ICE_API IncomingAsync final : public IncomingBase
{
public:

    IncomingAsync(IncomingBase&);

    static std::shared_ptr<IncomingAsync> create(IncomingBase&);
};

}

#endif
