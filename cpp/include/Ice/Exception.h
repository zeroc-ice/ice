// Copyright (c) ZeroC, Inc.

#ifndef ICE_EXCEPTION_H
#define ICE_EXCEPTION_H

#include "Config.h"

#include <exception>
#include <ostream>

namespace Ice
{
    /// Abstract base class for all Ice exceptions. It has only two derived classes: LocalException and UserException.
    /// @headerfile Ice/Ice.h
    class ICE_API Exception : public std::exception
    {
    public:
        /// Default constructor.
        Exception() noexcept = default;

        /// Copy constructor. Initializes this exception with a copy of @p other.
        /// @param other The exception to copy from.
        Exception(const Exception& other) noexcept = default;

        /// Copy assignment operator. Assigns the contents of @p rhs to this exception.
        /// @param rhs The exception to copy from.
        /// @return A reference to this exception.
        Exception& operator=(const Exception& rhs) noexcept = default;

        // Need out-of-line virtual function to avoid weak vtable, which in turn requires the default constructor,
        // copy constructor, and copy assignment operator to be declared explicitly.
        ~Exception() override;

        /// Returns the type ID of this exception. This corresponds to the Slice type ID for Slice-defined exceptions,
        /// and to a similar fully scoped name for other exceptions. For example
        /// "::Ice::CommunicatorDestroyedException".
        /// @return The type ID of this exception
        [[nodiscard]] virtual const char* ice_id() const noexcept = 0;

        /// Outputs a description of this exception to a stream.
        /// @param os The output stream.
        virtual void ice_print(std::ostream& os) const = 0;
    };

    /// Outputs a description of an Ice exception to a stream by calling Exception::ice_print on this exception.
    /// @param os The output stream.
    /// @param exception The exception to describe.
    /// @return The output stream.
    inline std::ostream& operator<<(std::ostream& os, const Exception& exception)
    {
        exception.ice_print(os);
        return os;
    }
}

#endif
