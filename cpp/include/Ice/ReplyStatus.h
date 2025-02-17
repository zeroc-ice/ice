// Copyright (c) ZeroC, Inc.

#ifndef ICE_REPLY_STATUS_H
#define ICE_REPLY_STATUS_H

#include "Config.h"

#include <cstdint>
#include <ostream>

namespace Ice
{
    /**
     * Represents the status of a response.
     * \headerfile Ice/Ice.h
     */
    enum class ReplyStatus : std::uint8_t
    {
        Ok = 0,
        UserException = 1,
        ObjectNotExist = 2,
        FacetNotExist = 3,
        OperationNotExist = 4,
        UnknownLocalException = 5,
        UnknownUserException = 6,
        UnknownException = 7
    };

    ICE_API std::ostream& operator<<(std::ostream&, ReplyStatus);
}

#endif
