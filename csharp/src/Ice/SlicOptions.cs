// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    internal record SlicOptions
    {
        internal TimeSpan IdleTimeout { get; init; }

        internal int MaxBidirectionalStreams { get; init; }

        internal int MaxUnidirectionalStreams { get; init; }

        internal int PacketSize { get; init; }
    }
}
