// Copyright (c) ZeroC, Inc.

#ifndef ICE_MARSHALED_RESULT_H
#define ICE_MARSHALED_RESULT_H

#include "Config.h"
#include "Current.h"
#include "OutputStream.h"

namespace Ice
{
    /// Base class for marshaled result structures, which are generated for operations marked with
    /// "marshaled-result" metadata.
    /// @headerfile Ice/Ice.h
    class ICE_API MarshaledResult
    {
    public:
        MarshaledResult() = delete;
        MarshaledResult(const MarshaledResult&) = delete;

        /// Move constructor. Constructs a MarshaledResult with the contents of @p other using move semantics.
        /// @param other The result to move from.
        MarshaledResult(MarshaledResult&& other) = default;

        virtual ~MarshaledResult();

        MarshaledResult& operator=(const MarshaledResult&) = delete;

        /// Move assignment operator. Replaces the contents of this result with the contents of @p rhs using move
        /// semantics.
        /// @param rhs The result to move from.
        /// @return A reference to this result.
        MarshaledResult& operator=(MarshaledResult&& rhs) = default;

        /// @cond INTERNAL

        OutputStream&& outputStream() && noexcept { return std::move(_ostr); }

    protected:
        /// The constructor requires the Current object that was passed to the servant.
        MarshaledResult(const Current& current);

        /// The output stream used to marshal the results.
        OutputStream _ostr;

        /// @endcond
    };
}

#endif
