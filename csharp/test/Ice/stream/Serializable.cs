//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice.Test.Stream.Serialize
{
    [Serializable]
    public class Small // Fewer than 254 bytes with a BinaryFormatter.
    {
        public int i;
    }
}
