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

namespace IceInternal
{

public class SequencePatcher : Patcher
{
    public
    SequencePatcher(System.Object[] seq, System.Type cls, string type, int index)
    {
	_seq = seq;
	_cls = cls;
	_type = type;
	_index = index;
    }
    
    public virtual void
    patch(Ice.Object v)
    {
	//
	// Raise ClassCastException if the element doesn't match the expected type.
	//
	if (!_cls.IsInstanceOfType(v.GetType()))
	{
	    throw new System.InvalidCastException("expected element of type " + _cls.FullName + " but received " + v.GetType().FullName);
	}
	
	_seq[_index] = v;
    }
    
    public virtual string
    type()
    {
	return _type;
    }
    
    private System.Object[] _seq;
    private System.Type _cls;
    private string _type;
    private int _index;
}

}
