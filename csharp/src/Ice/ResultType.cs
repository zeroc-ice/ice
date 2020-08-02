//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>The result type used in ice2 response frames.</summary>
    // Note: the values Success and Failure match (and must match) the values of ReplyStatus.OK and
    // ReplyStatus.UserException.
    public enum ResultType : byte
    {
        Success = 0,
        Failure = 1
    }
}
