// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


namespace IceInternal
{

    using System.Collections;

    public class ListPatcher : Patcher
    {
	public ListPatcher(ArrayList list, System.Type cls, string type, int index)
	{
	    _list = list;
	    _cls = cls;
	    _type = type;
	    _index = index;
	}
	
	public virtual void patch(Ice.Object v)
	{
	    //
	    // Raise InvalidCastException if the element doesn't match the expected type.
	    //
	    if(!_cls.IsInstanceOfType(v.GetType()))
	    {
		throw new System.InvalidCastException("expected element of type " + _cls.FullName
						      + " but received " + v.GetType().FullName);
	    }
	    
	    //
	    // This isn't very efficient for sequentially-accessed lists, but there
	    // isn't much we can do about it as long as a new patcher instance is
	    // created for each element.
	    //
	    _list[_index] = v;
	}
	
	public virtual string type()
	{
	    return _type;
	}
	
	private ArrayList _list;
	private System.Type _cls;
	private string _type;
	private int _index;
    }

}
