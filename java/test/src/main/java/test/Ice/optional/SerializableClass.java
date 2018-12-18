// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.optional;

public class SerializableClass implements java.io.Serializable
{
    public SerializableClass(int v)
    {
        _v = v;
    }

    @Override
    public boolean equals(Object obj)
    {
        if(obj instanceof SerializableClass)
        {
            return _v == ((SerializableClass)obj)._v;
        }

        return false;
    }

    private int _v;
    public static final long serialVersionUID = 1;
}
