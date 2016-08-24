// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

public class DictionaryPatcher<K, V> implements com.zeroc.Ice.ReadValueCallback
{
    public DictionaryPatcher(java.util.Map<K, V> dict, Class<V> cls, String type, K key)
    {
        _dict = dict;
        _cls = cls;
        _type = type;
        _key = key;
    }

    public void valueReady(com.zeroc.Ice.Value v)
    {
        if(v == null || _cls.isInstance(v))
        {
            _dict.put(_key, _cls.cast(v));
        }
        else
        {
            Ex.throwUOE(_type, v);
        }
    }

    private java.util.Map<K, V> _dict;
    private Class<V> _cls;
    private String _type;
    private K _key;
}
