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

    public sealed class DefaultsAndOverrides
    {
	internal DefaultsAndOverrides(Ice.Properties properties)
	{
	    string val;
	    
	    defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
	    
	    val = properties.getProperty("Ice.Default.Host");
	    if(val.Length != 0)
	    {
		defaultHost = val;
	    }
	    else
	    {
		defaultHost = Network.getLocalHost(true);
	    }
	    
	    defaultRouter = properties.getProperty("Ice.Default.Router");
	    
	    val = properties.getProperty("Ice.Override.Timeout");
	    if(val.Length > 0)
	    {
		overrideTimeout = true;
		overrideTimeoutValue = properties.getPropertyAsInt("Ice.Override.Timeout");
	    }
	    else
	    {
		overrideTimeout = false;
		overrideTimeoutValue = -1;
	    }
	    
	    val = properties.getProperty("Ice.Override.ConnectTimeout");
	    if(val.Length > 0)
	    {
	        overrideConnectTimeout = true;
		overrideConnectTimeoutValue = properties.getPropertyAsInt("Ice.Override.ConnectTimeout");
	    }
	    else
	    {
	        overrideTimeout = false;
		overrideTimeoutValue = -1;
	    }

	    val = properties.getProperty("Ice.Override.Compress");
	    if(val.Length > 0)
	    {
		overrideCompress = true;
		overrideCompressValue = properties.getPropertyAsInt("Ice.Override.Compress") != 0;
	    }
	    else
	    {
		overrideCompress = false;
		overrideCompressValue = false;
	    }
	    
	    defaultLocator = properties.getProperty("Ice.Default.Locator");
	}
	
	public string defaultHost;
	public string defaultProtocol;
	public string defaultRouter;
	public string defaultLocator;
	
	public bool overrideTimeout;
	public int overrideTimeoutValue;
	public bool overrideConnectTimeout;
	public int overrideConnectTimeoutValue;
	public bool overrideCompress;
	public bool overrideCompressValue;
    }

}
