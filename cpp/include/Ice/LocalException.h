//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCAL_EXCEPTION_H
#define ICE_LOCAL_EXCEPTION_H

#include "Exception.h"

namespace Ice
{
    /**
     * Base class for all Ice exceptions not defined in Slice.
     * \headerfile Ice/Ice.h
     */
    class ICE_API LocalException : public Exception
    {
    public:
        /**
         * Constructs a local exception.
         * @param file The file where this exception is constructed. This C string is not copied.
         * @param line The line where this exception is constructed.
         * @param message The error message adopted by this exception and returned by what().
         */
        LocalException(const char* file, int line, std::string message) : Exception(file, line, std::move(message)) {}

        [[nodiscard]] const char* ice_id() const noexcept override;
    };
}

#endif
