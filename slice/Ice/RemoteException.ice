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
    struct RemoteExceptionOrigin
    {
       Identity identity;
       string facet;
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
