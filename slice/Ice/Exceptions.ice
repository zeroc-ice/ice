//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export:ICE_API]]
[[cpp:doxygen:include:Ice/Ice.h]]
[[cpp:header-ext:h]]

[[suppress-warning:reserved-identifier]]
[[js:module:ice]]

[[python:pkgdir:Ice]]

[[java:package:com.zeroc]]

#include <Ice/Identity.ice>

[cs:namespace:ZeroC]
module Ice
{
    /// Represents the origin of a remote exception, with ice2 all remote exceptions have an origin field set, with
    /// ice1 it is only set for {@link ObjectNotExistException} and {@link OperationNotExistException}.
    [cs:readonly] struct RemoteExceptionOrigin
    {
        /// The Ice object Identity.
        Identity identity;
        /// The Ice object facet.
        string facet;
        /// The operation name.
        string operation;
    }

#ifdef __SLICE2CS__
    /// With ice1, an ObjectNotExistException is transmitted without the message string.
    exception ObjectNotExistException
    {
    }

    /// With ice1, an OperationNotExistException is transmitted without the message string.
    exception OperationNotExistException
    {
    }

    /// An unhandled exception is thrown when an operation implementation throws an exception not derived from
    /// RemoteException or when it throws a RemoteException with its convertToUnhandled flag set to true.
    /// With ice1, an UnhandledException is transmitted as an "UnknownLocalException" with just a string (the message)
    /// as its payload. When receiving any Unknown exception over ice1, the mapped exception is UnhandledException.
    exception UnhandledException
    {
    }
#endif
}
