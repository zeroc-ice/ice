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

package Complex;

public class ObjectFactoryI extends Ice.LocalObjectImpl implements Ice.ObjectFactory
{
    public Ice.Object
    create(String type)
    {
	if(type.equals("::Complex::MultiplyNode"))
	{
	    return new MultiplyNodeI();
	}
	if(type.equals("::Complex::AddNode"))
	{
	    return new AddNodeI();
	}
        if(type.equals("::Complex::NumberNode"))
	{
	    return new NumberNodeI();
	}

	System.err.println( "create: " + type);
	assert(false);
	return null;
    }

    public void
    destroy()
    {
	// Nothing to do
    }
}
