// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class SequencePatcher<T> implements com.zeroc.Ice.ReadValueCallback
{
    public SequencePatcher(T[] seq, Class<T> cls, String type, int index)
    {
        _seq = seq;
        _cls = cls;
        _type = type;
        _index = index;
    }

    public void valueReady(com.zeroc.Ice.Value v)
    {
        if(v == null || _cls.isInstance(v))
        {
            _seq[_index] = _cls.cast(v);
        }
        else
        {
            Ex.throwUOE(_type, v);
        }

    }

    private T[] _seq;
    private Class<T> _cls;
    private String _type;
    private int _index;
}
