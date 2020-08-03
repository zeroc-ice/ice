//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>The reply status of an ice1 response frame.</summary>
    public enum ReplyStatus : byte
    {
        OK = 0,
        UserException = 1,
        ObjectNotExistException = 2,
        FacetNotExistException = 3,
        OperationNotExistException = 4,
        UnknownLocalException = 5,
        UnknownUserException = 6,
        UnknownException = 7
    }
}
