// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

	value = properties.getProperty("Ice.Override.ConnectTimeout");
	if(value.length() > 0)
	{
	    overrideConnectTimeout = true;
	    overrideConnectTimeoutValue = properties.getPropertyAsInt("Ice.Override.ConnectTimeout");
	}
	else
	{
	    overrideConnectTimeout = false;
	    overrideConnectTimeoutValue = -1;
	}

	value = properties.getProperty("Ice.Override.Compress");
	if(value.length() > 0)
	{
	    overrideCompress = true;
	    boolean b = properties.getPropertyAsInt("Ice.Override.Compress") > 0;
	    if(!BasicStream.compressible() && b)
	    {
		System.err.println("warning: bzip2 support not available, Ice.Override.Compress ignored");
		b = false;
	    }
	    overrideCompressValue = b;
	}
	else
	{
	    overrideCompress = !BasicStream.compressible();
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
    final public boolean overrideConnectTimeout;
    final public int overrideConnectTimeoutValue;
    final public boolean overrideCompress;
    final public boolean overrideCompressValue;
}
