// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Complex;

public class ObjectFactoryI implements Ice.ObjectFactory
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
