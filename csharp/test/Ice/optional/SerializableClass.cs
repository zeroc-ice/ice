// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#if !SILVERLIGHT

using System;

[assembly: CLSCompliant(true)]

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

#endif
