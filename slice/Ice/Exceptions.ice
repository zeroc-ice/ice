//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#pragma once

[[cpp:dll-export(ICE_API)]]
[[cpp:doxygen:include(Ice/Ice.h)]]
[[cpp:header-ext(h)]]

[[suppress-warning(reserved-identifier)]]
[[js:module(ice)]]

[[python:pkgdir(Ice)]]

[[java:package(com.zeroc)]]

#include <Ice/Identity.ice>

[cs:namespace(ZeroC)]
module Ice
{
    /// Represents the origin of a remote exception. With the Ice 2.0 encoding, all remote exceptions have an implicit
    /// origin data member set during marshaling. With the Ice 1.1 encoding, this origin data member is only set and
    /// marshaled for {@link ObjectNotExistException} and {@link OperationNotExistException}.
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
    /// The object adapter could not find a servant for the target object.
    exception ObjectNotExistException
    {
    }

    /// The object adapter found a servant for the target object but this servant does not implement the requested
    /// operation. This exception is typically thrown when a client with newer Slice definitions calls an implementation
    /// based on older Slice definitions.
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

    /// One of the arguments to the operation has an invalid value.
    exception InvalidArgumentException
    {
        /// The name of the corresponding parameter.
        string paramName;
    }

    /// A server exception is thrown when an implementation error occured. This can occur for example if the
    /// response can't be sent because it's larger than the peer's incoming frame maximum size.
    exception ServerException
    {
    }
}
