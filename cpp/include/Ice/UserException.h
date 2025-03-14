// Copyright (c) ZeroC, Inc.

#ifndef ICE_USER_EXCEPTION_H
#define ICE_USER_EXCEPTION_H

#include "Exception.h"

namespace Ice
{
    class InputStream;
    class OutputStream;

    /// Abstract base class for all Ice exceptions defined in Slice.
    /// \headerfile Ice/Ice.h
    class ICE_API UserException : public Exception
    {
    public:
        /// Throws this exception.
        virtual void ice_throw() const = 0;

        /// Gets the Slice type ID of this user exception.
        /// @return The Slice type ID.
        [[nodiscard]] const char* what() const noexcept final;

        void ice_print(std::ostream& os) const override;

        /// Outputs the name and value of each field of this instance, including inherited fields, to the stream.
        /// @param os The output stream.
        virtual void ice_printFields(std::ostream& os) const;

        // _write and _read are virtual for the Python, Ruby etc. mappings.

        /// @private
        virtual void _write(OutputStream*) const;

        /// @private
        virtual void _read(InputStream*);

        /// @private
        [[nodiscard]] virtual bool _usesClasses() const;

    protected:
        /// @private
        virtual void _writeImpl(OutputStream*) const = 0;

        /// @private
        virtual void _readImpl(InputStream*) = 0;
    };
}

#endif
