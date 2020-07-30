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
#ifdef __SLICE2CS__

    /// A dispatch exception is a remote exception that reports an error during the server-side dispatch of a request.
    /// It is usually thrown by Ice but can also be thrown by application code.
    exception DispatchException
    {
        Identity id;
        string facet;
        string operation;
    }

    /// A pre-execution exception reports an error that occurred before the implementation of an operation. A client can
    /// safely retry a request that failed with a pre-execution exception; however, such retry may not be useful.
    exception PreExecutionException : DispatchException
    {
    }

    /// With ice1, an ObjectNotExistException is transmitted without the message string.
    exception ObjectNotExistException : PreExecutionException
    {
    }

    /// With ice1, an OperationNotExistException is transmitted without the message string.
    exception OperationNotExistException : PreExecutionException
    {
    }

    /// An unhandled exception is thrown when an operation implementation throws an exception not derived from
    /// RemoteException or when it throws a RemoteException with its convertToUnhandled flag set to true.
    /// With ice1, an UnhandledException is transmitted as an "UnknownLocalException" with just a string (the message)
    /// as its payload. When receiving any Unknown exception over ice1, the mapped exception is UnhandledException.
    exception UnhandledException : DispatchException
    {
    }
#endif
}
