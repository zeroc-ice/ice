//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["ice-prefix"]]
[["normalize-case"]]
[["js:module:ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

#include <Ice/Identity.ice>

module Ice
{
#ifdef __SLICE2CS__
    exception RequestFailedException
    {
        Identity id;
        string facet;
        string operation;
    }

    // A dispatch exception is an exception that occurs during the server-side request dispatch before the operation
    // implementation is called.
    exception DispatchException : RequestFailedException
    {
    }

    // With ice1, an ObjectNotExistException is transmitted without the message string.
    exception ObjectNotExistException : DispatchException
    {
    }

    // With ice1, an OperationNotExistException is transmitted without the message string.
    exception OperationNotExistException : DispatchException
    {
    }

    // An unhandled exception is thrown when an operation implementation throws an exception not derived from
    // RemoteException or when it throws a RemoteException with its convertToUnhandled flag set to true.
    // With ice1, an UnhandledException is transmitted as an "UnknownLocalException" with just a string (the message)
    // as its payload. When receiving any Unknown exception over ice1, the mapped exception is UnhandledException.
    exception UnhandledException : RequestFailedException
    {
    }
#endif
}
