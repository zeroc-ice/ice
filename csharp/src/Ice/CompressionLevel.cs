// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    /// <summary>Specifies values that indicate whether a compression operation emphasizes speed or compression size.
    /// </summary>
    public enum CompressionLevel : byte
    {
        /// <summary>The compression operation should complete as quickly as possible.</summary>
        Fastest = 0,
        /// <summary>The compression operation should be optimal, even if the operation takes a longer time to
        /// complete.</summary>
        Optimal = 1
    }
}
