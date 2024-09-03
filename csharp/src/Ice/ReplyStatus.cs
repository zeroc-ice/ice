// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents the status of a reply.
/// </summary>
public enum ReplyStatus : byte
{
    /// <summary>
    /// A successful reply message.
    /// </summary>
    Ok = 0,

    /// <summary>
    /// A user exception reply message.
    /// </summary>
    UserException = 1,

    /// <summary>
    /// The target object does not exist.
    /// </summary>
    ObjectNotExist = 2,

    /// <summary>
    /// The target object does not support the facet.
    /// </summary>
    FacetNotExist = 3,

    /// <summary>
    /// The target object does not support the operation.
    /// </summary>
    OperationNotExist = 4,

    /// <summary>
    /// The dispatch failed with an Ice local exception.
    /// </summary>
    UnknownLocalException = 5,

    /// <summary>
    /// The dispatch failed with a Slice user exception that does not conform to the exception specification of the
    /// operation.
    /// </summary>
    UnknownUserException = 6,

    /// <summary>
    /// The dispatch failed with some other exception.
    /// </summary>
    UnknownException = 7
}
