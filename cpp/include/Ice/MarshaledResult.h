//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_MARSHALED_RESULT_H
#define ICE_MARSHALED_RESULT_H

#include "Config.h"
#include "Current.h"
#include "OutputStream.h"

namespace IceInternal
{
    class Incoming;
}

namespace Ice
{

/**
 * Base class for marshaled result structures, which are generated for operations having the marshaled-result metadata
 * tag.
 * \headerfile Ice/Ice.h
 */
class ICE_API MarshaledResult
{
public:

    MarshaledResult() = delete;
    MarshaledResult(const MarshaledResult&) = delete;
    MarshaledResult(MarshaledResult&&);

    virtual ~MarshaledResult() = default;

    MarshaledResult& operator=(const MarshaledResult&) = delete;
    MarshaledResult& operator=(MarshaledResult&&);

protected:

    /// \cond INTERNAL

    /**
     * The constructor requires the Current object that was passed to the servant.
     */
    MarshaledResult(const Current& current);

    /** The output stream used to marshal the results. */
    OutputStream _ostr;

private:

    friend class IceInternal::Incoming;

    /**
     * Swaps the output stream of this object with the supplied output stream.
     * @param other The output stream to swap with.
     */
    void swap(OutputStream& other);

    /// \endcond
};

}

#endif
