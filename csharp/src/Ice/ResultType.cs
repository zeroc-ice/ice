//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>When Ice receives a response frame with an encapsulation encoded using the 2.0 encoding, the byte
    /// immediately after the compression byte in the decompressed encapsulation payload contains the result type.
    /// </summary>
    // The values are consistent with the ReplyStatus enumerators.
    public enum ResultType : byte
    {
        Success = 0,
        Failure = 1
    }
}
