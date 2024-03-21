//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_ENCODING_VERSION_H
#define ICE_ENCODING_VERSION_H

#include "Config.h"
#include "Comparable.h"
#include <cstdint>

namespace Ice
{
    /**
     * Represents a version of the Slice encoding.
     * \headerfile Ice/Ice.h
     */
    struct EncodingVersion
    {
        std::uint8_t major;
        std::uint8_t minor;

        /**
         * Obtains a tuple containing all of the struct's data members.
         * @return The data members in a tuple.
         */
        std::tuple<const std::uint8_t&, const std::uint8_t&> ice_tuple() const
        {
            return std::tie(major, minor);
        }
    };

    using Ice::operator<;
    using Ice::operator<=;
    using Ice::operator>;
    using Ice::operator>=;
    using Ice::operator==;
    using Ice::operator!=;
}

#endif
