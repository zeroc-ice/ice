// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
	    overrideComppress = true;
	    overrideComppressValue = properties.getPropertyAsInt("Ice.Override.Compress") != 0;
	}
	else
	{
	    overrideComppress = false;
	    overrideComppressValue = false;
	}

	defaultLocator = properties.getProperty("Ice.Default.Locator");
    }

    final public String defaultHost;
    final public String defaultProtocol;
    final public String defaultRouter;
    final public String defaultLocator;

    final public boolean overrideTimeout;
    final public int overrideTimeoutValue;
    final public boolean overrideComppress;
    final public boolean overrideComppressValue;
}
