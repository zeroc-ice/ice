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

public sealed class ServantLocatorI : Ice.LocalObjectImpl, Ice.ServantLocator
{
    public Ice.Object locate(Ice.Current curr, out Ice.LocalObject cookie)
    {
	cookie = null;
	return null;
    }
    
    public void finished(Ice.Current curr, Ice.Object servant, Ice.LocalObject cookie)
    {
    }
    
    public void deactivate(string category)
    {
    }
}
