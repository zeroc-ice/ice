// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

public final class DefaultsAndOverrides
{
    DefaultsAndOverrides(Ice.Properties properties, Ice.Logger logger)
    {
        String value;
        int intValue;

        defaultProtocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");

        value = properties.getProperty("Ice.Default.Host");
        if(!value.isEmpty())
        {
            defaultHost = value;
        }
        else
        {
            defaultHost = null;
        }

        value = properties.getProperty("Ice.Default.SourceAddress");
        if(!value.isEmpty())
        {
            defaultSourceAddress = Network.getNumericAddress(value);
            if(defaultSourceAddress == null)
            {
                throw new Ice.InitializationException("invalid IP address set for Ice.Default.SourceAddress: `" +
                                                      value + "'");
            }
        }
        else
        {
            defaultSourceAddress = null;
        }

        value = properties.getProperty("Ice.Override.Timeout");
        if(!value.isEmpty())
        {
            overrideTimeout = true;
            intValue = properties.getPropertyAsInt("Ice.Override.Timeout");
            if(intValue < 0 && intValue != -1)
            {
                overrideTimeoutValue = -1;
                StringBuffer msg = new StringBuffer("invalid value for Ice.Override.Timeout `");
                msg.append(properties.getProperty("Ice.Override.Timeout"));
                msg.append("': defaulting to -1");
                logger.warning(msg.toString());
            }
            else
            {
               overrideTimeoutValue = intValue;
            }
        }
        else
        {
            overrideTimeout = false;
            overrideTimeoutValue = -1;
        }

        value = properties.getProperty("Ice.Override.ConnectTimeout");
        if(!value.isEmpty())
        {
            overrideConnectTimeout = true;
            intValue = properties.getPropertyAsInt("Ice.Override.ConnectTimeout");
            if(intValue < 0 && intValue != -1)
            {
                overrideConnectTimeoutValue = -1;
                StringBuffer msg = new StringBuffer("invalid value for Ice.Override.ConnectTimeout `");
                msg.append(properties.getProperty("Ice.Override.ConnectTimeout"));
                msg.append("': defaulting to -1");
                logger.warning(msg.toString());
            }
            else
            {
                overrideConnectTimeoutValue = intValue;
            }
        }
        else
        {
            overrideConnectTimeout = false;
            overrideConnectTimeoutValue = -1;
        }

        value = properties.getProperty("Ice.Override.CloseTimeout");
        if(!value.isEmpty())
        {
            overrideCloseTimeout = true;
            intValue = properties.getPropertyAsInt("Ice.Override.CloseTimeout");
            if(intValue < 0 && intValue != -1)
            {
                overrideCloseTimeoutValue = -1;
                StringBuffer msg = new StringBuffer("invalid value for Ice.Override.CloseTimeout `");
                msg.append(properties.getProperty("Ice.Override.CloseTimeout"));
                msg.append("': defaulting to -1");
                logger.warning(msg.toString());
            }
            else
            {
                overrideCloseTimeoutValue = intValue;
            }
        }
        else
        {
            overrideCloseTimeout = false;
            overrideCloseTimeoutValue = -1;
        }

        value = properties.getProperty("Ice.Override.Compress");
        if(!value.isEmpty())
        {
            overrideCompress = true;
            boolean b = properties.getPropertyAsInt("Ice.Override.Compress") > 0;
            if(b && !BZip2.supported())
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
        if(!value.isEmpty())
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

        intValue = properties.getPropertyAsIntWithDefault("Ice.Default.Timeout", 60000);
        if(intValue < 1 && intValue != -1)
        {
            defaultTimeout = 60000;
            StringBuffer msg = new StringBuffer("invalid value for Ice.Default.Timeout `");
            msg.append(properties.getProperty("Ice.Default.Timeout"));
            msg.append("': defaulting to 60000");
            logger.warning(msg.toString());
        }
        else
        {
            defaultTimeout = intValue;
        }

        intValue = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);
        if(intValue < -1)
        {
            defaultLocatorCacheTimeout = -1;
            StringBuffer msg = new StringBuffer("invalid value for Ice.Default.LocatorCacheTimeout `");
            msg.append(properties.getProperty("Ice.Default.LocatorCacheTimeout"));
            msg.append("': defaulting to -1");
            logger.warning(msg.toString());
        }
        else
        {
            defaultLocatorCacheTimeout = intValue;
        }

        intValue = properties.getPropertyAsIntWithDefault("Ice.Default.InvocationTimeout", -1);
        if(intValue < 1 && intValue != -1 && intValue != -2)
        {
            defaultInvocationTimeout = -1;
            StringBuffer msg = new StringBuffer("invalid value for Ice.Default.InvocationTimeout `");
            msg.append(properties.getProperty("Ice.Default.InvocationTimeout"));
            msg.append("': defaulting to -1");
            logger.warning(msg.toString());
        }
        else
        {
            defaultInvocationTimeout = intValue;
        }

        defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

        value = properties.getPropertyWithDefault("Ice.Default.EncodingVersion",
                                                  Ice.Util.encodingVersionToString(Protocol.currentEncoding));
        defaultEncoding = Ice.Util.stringToEncodingVersion(value);
        Protocol.checkSupportedEncoding(defaultEncoding);

        boolean slicedFormat = properties.getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
        defaultFormat = slicedFormat ? Ice.FormatType.SlicedFormat : Ice.FormatType.CompactFormat;
    }

    final public String defaultHost;
    final public java.net.InetSocketAddress defaultSourceAddress;
    final public String defaultProtocol;
    final public boolean defaultCollocationOptimization;
    final public Ice.EndpointSelectionType defaultEndpointSelection;
    final public int defaultTimeout;
    final public int defaultLocatorCacheTimeout;
    final public int defaultInvocationTimeout;
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
