// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    internal record SlicOptions
    {
        internal int MaxBidirectionalStreams { get; init; }

        internal int MaxUnidirectionalStreams { get; init; }

        internal int PacketMaxSize { get; init; }
    }
}
