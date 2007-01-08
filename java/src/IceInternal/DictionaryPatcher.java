// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class DictionaryPatcher implements Patcher, Ice.ReadObjectCallback
{
    public
    DictionaryPatcher(java.util.Map dict, Class cls, String type, java.lang.Object key)
    {
        _dict = dict;
        _cls = cls;
        _type = type;
        _key = key;
    }

    public void
    patch(Ice.Object v)
    {
        if(v != null)
        {
            //
            // Raise ClassCastException if the element doesn't match the expected type.
            //
            if(!_cls.isInstance(v))
            {
                throw new ClassCastException("expected element of type " + _cls.getName() + " but received " +
                                             v.getClass().getName());
            }
        }

        _dict.put(_key, v);
    }

    public String
    type()
    {
        return _type;
    }

    public void
    invoke(Ice.Object v)
    {
        patch(v);
    }

    private java.util.Map _dict;
    private Class _cls;
    private String _type;
    private java.lang.Object _key;
}
