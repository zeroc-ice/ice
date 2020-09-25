//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    internal class SlicOptions
    {
        internal int MaxBidirectionalStreams { get; set; } = 10;

        internal int MaxUnidirectionalStreams { get; set; } = 10;

        public TimeSpan IdleTimeout { get; set; } = TimeSpan.FromSeconds(30);
    }
}
