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

public final class DefaultsAndOverwrites
{
    DefaultsAndOverwrites(Ice.Properties properties)
    {
	String value;
	
	defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");

	value = properties.getProperty("Ice.Default.Host");
	if (value.length() != 0)
	{
	    defaultHost = value;
	}
	else
	{
	    defaultHost = Network.getLocalHost(true);
	}
	
	defaultRouter = properties.getProperty("Ice.Default.Router");
	
	value = properties.getProperty("Ice.Overwrite.Timeout");
	if (value.length() > 0)
	{
	    overwriteTimeout = true;
	    overwriteTimeoutValue = properties.getPropertyAsInt("Ice.Overwrite.Timeout");
	}
	else
	{
	    overwriteTimeout = false;
	    overwriteTimeoutValue = -1;
	}

	value = properties.getProperty("Ice.Overwrite.Compress");
	if (value.length() > 0)
	{
	    overwriteCompress = true;
	    overwriteCompressValue = properties.getPropertyAsInt("Ice.Overwrite.Compress") != 0;
	}
	else
	{
	    overwriteCompress = false;
	    overwriteCompressValue = false;
	}
    }

    final public String defaultHost;
    final public String defaultProtocol;
    final public String defaultRouter;

    final public boolean overwriteTimeout;
    final public int overwriteTimeoutValue;
    final public boolean overwriteCompress;
    final public boolean overwriteCompressValue;
}
