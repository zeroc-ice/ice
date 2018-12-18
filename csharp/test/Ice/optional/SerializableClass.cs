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
    namespace optional
    {
        namespace Test
        {
            [Serializable]
            public class SerializableClass
            {
                public SerializableClass(int v)
                {
                    _v = v;
                }

                public override bool Equals(object obj)
                {
                    if(obj is SerializableClass)
                    {
                        return _v == (obj as SerializableClass)._v;
                    }

                    return false;
                }

                public override int GetHashCode()
                {
                    return base.GetHashCode();
                }

                private int _v;
            }

        }
    }
}
