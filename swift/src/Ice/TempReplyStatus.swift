// Copyright (c) ZeroC, Inc.

/// Represents the status of a reply.
public enum TempReplyStatus: UInt8 {
    case ok = 0
    case userException = 1
    case objectNotExist = 2
    case facetNotExist = 3
    case operationNotExist = 4
    case unknownLocalException = 5
    case unknownUserException = 6
    case unknownException = 7
}
