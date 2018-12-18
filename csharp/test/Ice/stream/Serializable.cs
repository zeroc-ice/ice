// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System;

namespace Ice
{
    namespace stream
    {
        namespace Serialize
        {

            [Serializable]
            public class Small // Fewer than 254 bytes with a BinaryFormatter.
            {
                public int i;
            }

        }
    }
}
