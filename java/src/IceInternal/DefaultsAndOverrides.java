// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
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
            defaultHost = null;
        }
        
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

        value = properties.getProperty("Ice.Override.CloseTimeout");
        if(value.length() > 0)
        {
            overrideCloseTimeout = true;
            overrideCloseTimeoutValue = properties.getPropertyAsInt("Ice.Override.CloseTimeout");
        }
        else
        {
            overrideCloseTimeout = false;
            overrideCloseTimeoutValue = -1;
        }

        value = properties.getProperty("Ice.Override.Compress");
        if(value.length() > 0)
        {
            overrideCompress = true;
            boolean b = properties.getPropertyAsInt("Ice.Override.Compress") > 0;
            if(b && !BasicStream.compressible())
            {
                System.err.println("warning: bzip2 support not available, Ice.Override.Compress ignored");
                b = false;
            }
            overrideCompressValue = b;
        }
        else
        {
            overrideCompress = false;
            overrideCompressValue = false;
        }

        value = properties.getProperty("Ice.Override.Secure");
        if(value.length() > 0)
        {
            overrideSecure = true;
            overrideSecureValue = properties.getPropertyAsInt("Ice.Override.Secure") > 0;
        }
        else
        {
            overrideSecure = false;
            overrideSecureValue = false;
        }

        defaultCollocationOptimization =
            properties.getPropertyAsIntWithDefault("Ice.Default.CollocationOptimized", 1) > 0;

        value = properties.getPropertyWithDefault("Ice.Default.EndpointSelection", "Random");
        if(value.equals("Random"))
        {
            defaultEndpointSelection = Ice.EndpointSelectionType.Random;
        }
        else if(value.equals("Ordered"))
        {
            defaultEndpointSelection = Ice.EndpointSelectionType.Ordered;
        }
        else
        {
            Ice.EndpointSelectionTypeParseException ex = new Ice.EndpointSelectionTypeParseException();
            ex.str = "illegal value `" + value + "'; expected `Random' or `Ordered'";
            throw ex;
        }

        defaultLocatorCacheTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);

        defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

        value = properties.getPropertyWithDefault("Ice.Default.EncodingVersion", 
                                                  Ice.Util.encodingVersionToString(Protocol.currentEncoding));
        defaultEncoding = Ice.Util.stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(defaultEncoding);        

        boolean slicedFormat = properties.getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
        defaultFormat = slicedFormat ? Ice.FormatType.SlicedFormat : Ice.FormatType.CompactFormat;
    }

    final public String defaultHost;
    final public String defaultProtocol;
    final public boolean defaultCollocationOptimization;
    final public Ice.EndpointSelectionType defaultEndpointSelection;
    final public int defaultLocatorCacheTimeout;
    final public boolean defaultPreferSecure;
    final public Ice.EncodingVersion defaultEncoding;
    final public Ice.FormatType defaultFormat;

    final public boolean overrideTimeout;
    final public int overrideTimeoutValue;
    final public boolean overrideConnectTimeout;
    final public int overrideConnectTimeoutValue;
    final public boolean overrideCloseTimeout;
    final public int overrideCloseTimeoutValue;
    final public boolean overrideCompress;
    final public boolean overrideCompressValue;
    final public boolean overrideSecure;
    final public boolean overrideSecureValue;
}
