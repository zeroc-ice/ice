// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
