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

package IceInternal;

public final class DefaultsAndOverrides
{
    DefaultsAndOverrides(Ice.Properties properties)
    {
	String value;
	
	defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");

	value = properties.getProperty("Ice.Default.Host");
	if(value.length() != 0)
	{
	    defaultHost = value;
	}
	else
	{
	    defaultHost = Network.getLocalHost(true);
	}
	
	defaultRouter = properties.getProperty("Ice.Default.Router");
	
	value = properties.getProperty("Ice.Override.Timeout");
	if(value.length() > 0)
	{
	    overrideTimeout = true;
	    overrideTimeoutValue = properties.getPropertyAsInt("Ice.Override.Timeout");
	}
	else
	{
	    overrideTimeout = false;
	    overrideTimeoutValue = -1;
	}

	value = properties.getProperty("Ice.Override.Compress");
	if(value.length() > 0)
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

    final public String defaultHost;
    final public String defaultProtocol;
    final public String defaultRouter;
    final public String defaultLocator;

    final public boolean overrideTimeout;
    final public int overrideTimeoutValue;
    final public boolean overrideCompress;
    final public boolean overrideCompressValue;
}
