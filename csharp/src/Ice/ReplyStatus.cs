// Copyright (c) ZeroC, Inc.

#nullable enable

namespace Ice;

/// <summary>
/// Represents the status of a reply.
/// </summary>
public enum ReplyStatus : byte
{
    Ok = 0,
    UserException = 1,
    ObjectNotExist = 2,
    FacetNotExist = 3,
    OperationNotExist = 4,
    UnknownLocalException = 5,
    UnknownUserException = 6,
    UnknownException = 7
}
