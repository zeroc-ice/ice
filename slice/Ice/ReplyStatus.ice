// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/StreamHelpers.h"]]

[["js:module:ice"]]

[["python:pkgdir:Ice"]]

[["java:package:com.zeroc"]]

module Ice
{
    /// Represents the status of a reply.
    enum ReplyStatus
    {
        /// The dispatch completed successfully.
        Ok = 0,

        /// The dispatch completed with a Slice user exception.
        UserException,

        /// The dispatch could not find an implementation for the target object.
        ObjectNotExist,

        /// The dispatch found an implementation for the target object but could not find the requested facet.
        FacetNotExist,

        /// The dispatch found an implementation for the target object but could not find the requested operation.
        OperationNotExist,

        /// The dispatch failed with an Ice local exception.
        UnknownLocalException,

        /// The dispatch failed with a Slice user exception that does not conform to the exception specification of
        /// the operation.
        UnknownUserException,

        /// The dispatch failed with some other exception (neither an Ice local exception nor a Slice user exception).
        UnknownException,

        /// The dispatch failed because the request payload could not be unmarshaled. It is typically due to a mismatch
        /// in the Slice definitions used by the client and the server.
        InvalidData,

        /// The caller is not authorized to access the requested resource.
        Unauthorized,

        // Work-around to get an "unchecked" enum in C++, C# and other languages.
        // 126 is the maximum value encoded on a single byte with the Ice 1.0 encoding - the encoding used when
        // marshaling reply headers.
        MaxValue = 126
    }
}
