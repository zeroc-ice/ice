// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
