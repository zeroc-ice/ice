//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#pragma once

#include <Ice/Ice.h>

namespace IceStorm
{
    class SendQueueSizeMaxReached : public Ice::LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static ::std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_throw() const override;
    };
}
