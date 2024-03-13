//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#pragma once

#include <Ice/Ice.h>

namespace IceGrid
{
    class SynchronizationException : public Ice::LocalException
    {
    public:
        using LocalException::LocalException;

        /**
         * Obtains a tuple containing all of the exception's data members.
         * @return The data members in a tuple.
         */
        std::tuple<> ice_tuple() const { return std::tie(); }

        /**
         * Obtains the Slice type ID of this exception.
         * @return The fully-scoped type ID.
         */
        static ::std::string_view ice_staticId() noexcept;

        std::string ice_id() const override;

        void ice_throw() const override;
    };
}
