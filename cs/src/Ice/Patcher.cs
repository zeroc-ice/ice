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

namespace Ice
{
    public interface SequenceBase
    {
	void set(int i, object v);
    }
}

namespace IceInternal
{
    public interface Patcher
    {
	void patch(Ice.Object v);
	string type();
    }

    public sealed class ParamPatcher : Patcher
    {
	public ParamPatcher(System.Type type)
	{
	    _type = type;
	}

	public void patch(Ice.Object v)
	{
	    value = v;
	}

	public string type()
	{
	    return _type.FullName;
	}

	public Ice.Object value;
	public System.Type _type;
    }

    public sealed class SequencePatcher : Patcher
    {
	public SequencePatcher(Ice.SequenceBase seq, System.Type type, int index)
	{
	    _seq = seq;
	    _type = type;
	    _index = index;
	}
    
	public void patch(Ice.Object v)
	{
	    //
	    // Raise InvalidCastException if the element doesn't match the expected type.
	    //
	    if(!(_type.IsAssignableFrom(v.GetType())))
	    {
		throw new System.InvalidCastException("expected element of type " + type() + " but received " + v.GetType().FullName);
	    }
	
	    _seq.set(_index, v);
	}

	public string type()
	{
	    return _type.FullName;
	}
    
	private Ice.SequenceBase _seq;
	private System.Type _type;
	private int _index;
    }
}
