// Copyright (c) ZeroC, Inc.

#pragma once

[["cpp:dll-export:ICE_API"]]
[["cpp:doxygen:include:Ice/Ice.h"]]
[["cpp:header-ext:h"]]

[["cpp:no-default-include"]]
[["cpp:include:Ice/Config.h"]]
[["cpp:include:Ice/StreamHelpers.h"]]

[["js:module:@zeroc/ice"]]

["java:identifier:com.zeroc.Ice"]
module Ice
{
    /// Represents the status of a reply.
    /// A reply status can have any value in the range 0..255. Do not use this enum to marshal or unmarshal a reply
    /// status unless you know its value corresponds to one of the enumerators defined below.
    enum ReplyStatus
    {
        /// The dispatch completed successfully.
        ["swift:identifier:ok"]
        Ok = 0,

        /// The dispatch completed with a Slice user exception.
        ["swift:identifier:userException"]
        UserException,

        /// The dispatch could not find an implementation for the target object.
        ["swift:identifier:objectNotExist"]
        ObjectNotExist,

        /// The dispatch found an implementation for the target object but could not find the requested facet.
        ["swift:identifier:facetNotExist"]
        FacetNotExist,

        /// The dispatch found an implementation for the target object but could not find the requested operation.
        ["swift:identifier:operationNotExist"]
        OperationNotExist,

        /// The dispatch failed with an Ice local exception.
        ["swift:identifier:unknownLocalException"]
        UnknownLocalException,

        /// The dispatch failed with a Slice user exception that does not conform to the exception specification of
        /// the operation.
        ["swift:identifier:unknownUserException"]
        UnknownUserException,

        /// The dispatch failed with some other exception (neither an Ice local exception nor a Slice user exception).
        ["swift:identifier:unknownException"]
        UnknownException,

        /// The dispatch failed because the request payload could not be unmarshaled. It is typically due to a mismatch
        /// in the Slice definitions used by the client and the server.
        ["swift:identifier:invalidData"]
        InvalidData,

        /// The caller is not authorized to access the requested resource.
        ["swift:identifier:unauthorized"]
        Unauthorized,
    }
}
