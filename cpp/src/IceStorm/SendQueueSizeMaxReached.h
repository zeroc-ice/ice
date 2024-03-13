//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

// TODO: rework this file.

#pragma once

#include <Ice/Ice.h>

namespace IceStorm
{
    class SendQueueSizeMaxReached : public ::Ice::LocalExceptionHelper<SendQueueSizeMaxReached, ::Ice::LocalException>
    {
    public:
        virtual ~SendQueueSizeMaxReached();

        SendQueueSizeMaxReached(const SendQueueSizeMaxReached&) = default;

        /**
         * The file and line number are required for all local exceptions.
         * @param file The file name in which the exception was raised, typically __FILE__.
         * @param line The line number at which the exception was raised, typically __LINE__.
         */
        SendQueueSizeMaxReached(const char* file, int line)
            : ::Ice::::Ice::LocalException(file, line)
        {
        }

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
    };
}
