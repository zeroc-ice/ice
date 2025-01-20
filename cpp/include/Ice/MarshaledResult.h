// Copyright (c) ZeroC, Inc.

#ifndef ICE_MARSHALED_RESULT_H
#define ICE_MARSHALED_RESULT_H

#include "Config.h"
#include "Current.h"
#include "OutputStream.h"

namespace Ice
{
    /**
     * Base class for marshaled result structures, which are generated for operations having the marshaled-result
     * metadata tag. \headerfile Ice/Ice.h
     */
    class ICE_API MarshaledResult
    {
    public:
        MarshaledResult() = delete;
        MarshaledResult(const MarshaledResult&) = delete;
        MarshaledResult(MarshaledResult&&) = default;

        virtual ~MarshaledResult() = default;

        MarshaledResult& operator=(const MarshaledResult&) = delete;
        MarshaledResult& operator=(MarshaledResult&&) = default;

        /// \cond INTERNAL

        OutputStream&& outputStream() && noexcept { return std::move(_ostr); }

    protected:
        /**
         * The constructor requires the Current object that was passed to the servant.
         */
        MarshaledResult(const Current& current);

        /** The output stream used to marshal the results. */
        OutputStream _ostr;

        /// \endcond
    };
}

#endif
