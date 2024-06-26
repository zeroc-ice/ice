//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_LOCAL_EXCEPTION_H
#define ICE_LOCAL_EXCEPTION_H

#include "Exception.h"

namespace Ice
{
    /**
     * Abstract base class for all Ice exceptions not defined in Slice.
     * \headerfile Ice/Ice.h
     */
    class ICE_API LocalException : public Exception
    {
    public:
        using Exception::Exception;
    };
}

#endif
