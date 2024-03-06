//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_MARSHALED_RESULT_H
#define ICE_MARSHALED_RESULT_H

#include "Config.h"
#include "Current.h"
#include "OutputStream.h"

namespace Ice
{

/**
 * Base class for marshaled result structures, which are generated for operations having the
 * marshaled-result metadata tag.
 * \headerfile Ice/Ice.h
 */
class ICE_API MarshaledResult
{
public:

    // TODO: this class needs a virtual destructor

    /**
     * The constructor requires the Current object that was passed to the servant.
     */
    MarshaledResult(const Current& current);

    /**
     * Obtains the output stream that is used to marshal the results.
     * @return The output stream.
     */
    std::shared_ptr<OutputStream> getOutputStream() const
    {
        return ostr;
    }

protected:

    /** The output stream used to marshal the results. */
    std::shared_ptr<OutputStream> ostr;
};

}

#endif
