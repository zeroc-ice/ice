// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class Patcher<T> implements com.zeroc.Ice.ReadValueCallback
{
    @FunctionalInterface
    public interface Callback<T>
    {
        void valueReady(T v);
    }

    public Patcher(Class<T> cls, String type)
    {
        this(cls, type, null);
    }

    public Patcher(Class<T> cls, String type, Callback<T> cb)
    {
        _cls = cls;
        _type = type;
        _cb = cb;
    }

    public void valueReady(com.zeroc.Ice.Value v)
    {
        if(v == null || _cls.isInstance(v))
        {
            value = _cls.cast(v);
            if(_cb != null)
            {
                _cb.valueReady(value);
            }
        }
        else
        {
            Ex.throwUOE(_type, v);
        }
    }

    private Class<T> _cls;
    private String _type;
    private Callback<T> _cb;
    public T value;
}
