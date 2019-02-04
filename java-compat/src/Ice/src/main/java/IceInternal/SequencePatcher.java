//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class SequencePatcher implements Ice.ReadValueCallback
{
    public SequencePatcher(java.lang.Object[] seq, Class<?> cls, int index)
    {
        _seq = seq;
        _cls = cls;
        _index = index;
    }

    public void valueReady(Ice.Object v)
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
    private int _index;
}
