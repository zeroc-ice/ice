// Copyright (c) ZeroC, Inc. All rights reserved.

namespace ZeroC.Ice
{
    // Definitions for the ice2 protocol.

    internal static class Ice2Definitions
    {
        internal static readonly Encoding Encoding = Encoding.V2_0;

        // ice2 frame types:
        internal enum FrameType : byte
        {
            Request = 0,
            Response = 2,
        }
    }
}
