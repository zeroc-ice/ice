//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    internal class SlicOptions
    {
        internal TimeSpan IdleTimeout { get; set; } = TimeSpan.FromSeconds(30);

        internal int MaxBidirectionalStreams { get; set; } = 100;

        internal int MaxUnidirectionalStreams { get; set; } = 100;

        internal int PacketSize { get; set; } = 32 * 1024;
    }
}
