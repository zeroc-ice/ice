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

    public class NonRepeatable : Ice.Exception
    {
	public NonRepeatable(Ice.LocalException ex)
	{
	    _ex = ex;
	}
	
	public virtual Ice.LocalException get()
	{
	    return _ex;
	}
	
	private Ice.LocalException _ex;
    }

}
