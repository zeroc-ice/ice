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

public class FacetI extends ServantI implements Test._FacetOperations
{
    FacetI(Test.Facet tie)
    {
	super(tie);
    }

    FacetI(Test.Servant tie, RemoteEvictorI remoteEvictor, Freeze.Evictor evictor, int value, String data)
    {
	super(tie, remoteEvictor, evictor, value);
	((Test.Facet)_tie).data = data;
    }
    
    public String
    getData(Ice.Current current)
    {
	synchronized(_tie)
	{
	    return ((Test.Facet)_tie).data;
	}
    }

    public void
    setData(String data, Ice.Current current)
    {
	synchronized(_tie)
	{
	    ((Test.Facet)_tie).data = data;
	}
    }
}
