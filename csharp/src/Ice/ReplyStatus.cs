// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents the status of a reply.
/// </summary>
public enum ReplyStatus : byte
{
    /// <summary>The dispatch completed successfully.</summary>
    Ok = 0,

    /// <summary>The dispatch completed with a Slice user exception.</summary>
    UserException = 1,

    /// <summary>The dispatch could not find an implementation for the target object.</summary>
    ObjectNotExist = 2,

    /// <summary>The dispatch found an implementation for the target object but could not find the requested facet.
    /// </summary>
    FacetNotExist = 3,

    /// <summary>The dispatch found an implementation for the target object but could not find the requested operation.
    /// </summary>
    OperationNotExist = 4,

    /// <summary>The dispatch failed with an Ice local exception.</summary>
    UnknownLocalException = 5,

    /// <summary>The dispatch failed with a Slice user exception that does not conform to the exception specification of
    /// the operation.</summary>
    UnknownUserException = 6,

    /// <summary>The dispatch failed with some other exception.</summary>
    UnknownException = 7,

    /// <summary>The caller is not authorized to access the requested resource.</summary>
    Unauthorized = 8,
}
