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

package Ice;

public final class ObjectHolder
{
    public
    ObjectHolder()
    {
    }

    public
    ObjectHolder(Ice.Object value)
    {
        this.value = value;
    }

    public class Patcher implements IceInternal.Patcher
    {
	public void
	patch(Ice.Object v)
	{
	    value = v;
	}

	public String
	type()
	{
	    return Ice.ObjectImpl.ice_staticId();
	}
    }

    public Patcher
    getPatcher()
    {
	return new Patcher();
    }

    public Ice.Object value;
}
