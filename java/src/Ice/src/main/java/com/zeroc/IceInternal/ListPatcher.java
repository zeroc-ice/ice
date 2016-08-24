// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class ListPatcher<T> implements com.zeroc.Ice.ReadValueCallback
{
    public ListPatcher(java.util.List<T> list, Class<T> cls, String type, int index)
    {
        _list = list;
        _cls = cls;
        _type = type;
        _index = index;
    }

    public void valueReady(com.zeroc.Ice.Value v)
    {
        if(v == null || _cls.isInstance(v))
        {
            //
            // This isn't very efficient for sequentially-accessed lists, but there
            // isn't much we can do about it as long as a new patcher instance is
            // created for each element.
            //
            _list.set(_index, _cls.cast(v));
        }
        else
        {
            Ex.throwUOE(_type, v);
        }
    }

    private java.util.List<T> _list;
    private Class<T> _cls;
    private String _type;
    private int _index;
}
