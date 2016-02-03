// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public class SequencePatcher implements Ice.ReadObjectCallback
{
    // TBD: Remove _type?
    public SequencePatcher(java.lang.Object[] seq, Class<?> cls, String type, int index)
    {
        _seq = seq;
        _cls = cls;
        _type = type;
        _index = index;
    }

    public void objectReady(Ice.Object v)
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

        _seq[_index] = v;
    }

    private java.lang.Object[] _seq;
    private Class<?> _cls;
    private String _type;
    private int _index;
}
