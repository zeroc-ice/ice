//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package IceInternal;

public class ListPatcher<T> implements Ice.ReadValueCallback
{
    public ListPatcher(java.util.List<T> list, Class<T> cls, int index)
    {
        _list = list;
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

        //
        // This isn't very efficient for sequentially-accessed lists, but there
        // isn't much we can do about it as long as a new patcher instance is
        // created for each element.
        //
        _list.set(_index, _cls.cast(v));
    }

    private java.util.List<T> _list;
    private Class<T> _cls;
    private int _index;
}
