// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>The type of result carried by a response frame.</summary>
    // Note: the values Success and Failure match (and must match) the values of ReplyStatus.OK and
    // ReplyStatus.UserException.
    public enum ResultType : byte
    {
        /// <summary>The request succeed.</summary>
        Success = 0,
        /// <summary>The request failed.</summary>
        Failure = 1
    }
}
