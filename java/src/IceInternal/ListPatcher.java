// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

package IceInternal;

public class ListPatcher implements Patcher
{
    public
    ListPatcher(java.util.List list, Class cls, String type, int index)
    {
        _list = list;
        _cls = cls;
        _type = type;
        _index = index;
    }

    public void
    patch(Ice.Object v)
    {
        //
        // Raise ClassCastException if the element doesn't match the expected type.
        //
        if(!_cls.isInstance(v))
        {
            throw new ClassCastException("expected element of type " + _cls.getName() + " but received " +
                                         v.getClass().getName());
        }

        //
        // This isn't very efficient for sequentially-accessed lists, but there
        // isn't much we can do about it as long as a new patcher instance is
        // created for each element.
        //
        _list.set(_index, v);
    }

    public String
    type()
    {
        return _type;
    }

    private java.util.List _list;
    private Class _cls;
    private String _type;
    private int _index;
}
