// Copyright (c) ZeroC, Inc.

#ifndef ICE_REPLY_STATUS_H
#define ICE_REPLY_STATUS_H

#include "Ice/Config.h"

namespace IceInternal
{
    static const std::uint8_t replyOK = 0;
    static const std::uint8_t replyUserException = 1;
    static const std::uint8_t replyObjectNotExist = 2;
    static const std::uint8_t replyFacetNotExist = 3;
    static const std::uint8_t replyOperationNotExist = 4;
    static const std::uint8_t replyUnknownLocalException = 5;
    static const std::uint8_t replyUnknownUserException = 6;
    static const std::uint8_t replyUnknownException = 7;
}

#endif
