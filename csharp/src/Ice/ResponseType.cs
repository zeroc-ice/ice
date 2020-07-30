//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>When Ice receives an ice2 response frame, the first byte of the encapsulation's payload contains the
    /// response type. This is true regardless of the encoding of this encapsulation.</summary>
    // The values are consistent with the ReplyStatus enumerators to reduce confusion.
    public enum ResponseType : byte
    {
        Success = 0,
        Failure = 1
    }
}
