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

public class SequencePatcher implements Patcher
{
    public
    SequencePatcher(java.lang.Object[] seq, Class cls, String type, int index)
    {
        _seq = seq;
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

        _seq[_index] = v;
    }

    public String
    type()
    {
        return _type;
    }

    private java.lang.Object[] _seq;
    private Class _cls;
    private String _type;
    private int _index;
}
