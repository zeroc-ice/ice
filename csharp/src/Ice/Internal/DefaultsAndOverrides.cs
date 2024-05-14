// Copyright (c) ZeroC, Inc.

using System.Net;
using System.Text;

namespace Ice.Internal;

public sealed class DefaultsAndOverrides
{
    internal DefaultsAndOverrides(Ice.Properties properties, Ice.Logger logger)
    {
        string val;

        defaultProtocol = properties.getIceProperty("Ice.Default.Protocol");

        val = properties.getIceProperty("Ice.Default.Host");
        if (val.Length != 0)
        {
            defaultHost = val;
        }
        else
        {
            defaultHost = null;
        }

        val = properties.getIceProperty("Ice.Default.SourceAddress");
        if (val.Length > 0)
        {
            defaultSourceAddress = Network.getNumericAddress(val);
            if (defaultSourceAddress == null)
            {
                throw new Ice.InitializationException("invalid IP address set for Ice.Default.SourceAddress: `" +
                                                      val + "'");
            }
        }
        else
        {
            defaultSourceAddress = null;
        }

        val = properties.getIceProperty("Ice.Override.Timeout");
        if (val.Length > 0)
        {
            overrideTimeout = true;
            overrideTimeoutValue = properties.getIcePropertyAsInt("Ice.Override.Timeout");
            if (overrideTimeoutValue < 1 && overrideTimeoutValue != -1)
            {
                overrideTimeoutValue = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Override.Timeout `");
                msg.Append(properties.getIceProperty("Ice.Override.Timeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }
        }
        else
        {
            overrideTimeout = false;
            overrideTimeoutValue = -1;
        }

        val = properties.getIceProperty("Ice.Override.ConnectTimeout");
        if (val.Length > 0)
        {
            overrideConnectTimeout = true;
            overrideConnectTimeoutValue = properties.getIcePropertyAsInt("Ice.Override.ConnectTimeout");
            if (overrideConnectTimeoutValue < 1 && overrideConnectTimeoutValue != -1)
            {
                overrideConnectTimeoutValue = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Override.ConnectTimeout `");
                msg.Append(properties.getIceProperty("Ice.Override.ConnectTimeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }
        }
        else
        {
            overrideConnectTimeout = false;
            overrideConnectTimeoutValue = -1;
        }

        val = properties.getIceProperty("Ice.Override.CloseTimeout");
        if (val.Length > 0)
        {
            overrideCloseTimeout = true;
            overrideCloseTimeoutValue = properties.getIcePropertyAsInt("Ice.Override.CloseTimeout");
            if (overrideCloseTimeoutValue < 1 && overrideCloseTimeoutValue != -1)
            {
                overrideCloseTimeoutValue = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Override.CloseTimeout `");
                msg.Append(properties.getIceProperty("Ice.Override.CloseTimeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }
        }
        else
        {
            overrideCloseTimeout = false;
            overrideCloseTimeoutValue = -1;
        }

        val = properties.getIceProperty("Ice.Override.Compress");
        if (val.Length > 0)
        {
            overrideCompress = true;
            overrideCompressValue = properties.getIcePropertyAsInt("Ice.Override.Compress") > 0;
            if (!BZip2.supported() && overrideCompressValue)
            {
                string lib = AssemblyUtil.isWindows ? "bzip2.dll" : "libbz2.so.1";
                Console.Error.WriteLine("warning: " + lib + " not found, Ice.Override.Compress ignored.");
                overrideCompressValue = false;
            }
        }
        else
        {
            overrideCompress = !BZip2.supported();
            overrideCompressValue = false;
        }

        val = properties.getIceProperty("Ice.Override.Secure");
        if (val.Length > 0)
        {
            overrideSecure = true;
            overrideSecureValue = properties.getIcePropertyAsInt("Ice.Override.Secure") > 0;
        }
        else
        {
            overrideSecure = false;
            overrideSecureValue = false;
        }

        defaultCollocationOptimization =
            properties.getIcePropertyAsInt("Ice.Default.CollocationOptimized") > 0;

        val = properties.getIceProperty("Ice.Default.EndpointSelection");
        if (val == "Random")
        {
            defaultEndpointSelection = Ice.EndpointSelectionType.Random;
        }
        else if (val == "Ordered")
        {
            defaultEndpointSelection = Ice.EndpointSelectionType.Ordered;
        }
        else
        {
            Ice.EndpointSelectionTypeParseException ex = new Ice.EndpointSelectionTypeParseException();
            ex.str = "illegal value `" + val + "'; expected `Random' or `Ordered'";
            throw ex;
        }

        defaultTimeout = properties.getIcePropertyAsInt("Ice.Default.Timeout");
        if (defaultTimeout < 1 && defaultTimeout != -1)
        {
            defaultTimeout = 60000;
            StringBuilder msg = new StringBuilder("invalid value for Ice.Default.Timeout `");
            msg.Append(properties.getIceProperty("Ice.Default.Timeout"));
            msg.Append("': defaulting to 60000");
            logger.warning(msg.ToString());
        }

        defaultLocatorCacheTimeout = properties.getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
        if (defaultLocatorCacheTimeout < -1)
        {
            defaultLocatorCacheTimeout = -1;
            StringBuilder msg = new StringBuilder("invalid value for Ice.Default.LocatorCacheTimeout `");
            msg.Append(properties.getIceProperty("Ice.Default.LocatorCacheTimeout"));
            msg.Append("': defaulting to -1");
            logger.warning(msg.ToString());
        }

        defaultInvocationTimeout = properties.getIcePropertyAsInt("Ice.Default.InvocationTimeout");
        if (defaultInvocationTimeout < 1 && defaultInvocationTimeout != -1 && defaultInvocationTimeout != -2)
        {
            defaultInvocationTimeout = -1;
            StringBuilder msg = new StringBuilder("invalid value for Ice.Default.InvocationTimeout `");
            msg.Append(properties.getIceProperty("Ice.Default.InvocationTimeout"));
            msg.Append("': defaulting to -1");
            logger.warning(msg.ToString());
        }

        defaultPreferSecure = properties.getIcePropertyAsInt("Ice.Default.PreferSecure") > 0;

        val = properties.getPropertyWithDefault("Ice.Default.EncodingVersion",
                                                Ice.Util.encodingVersionToString(Ice.Util.currentEncoding));
        defaultEncoding = Ice.Util.stringToEncodingVersion(val);
        Protocol.checkSupportedEncoding(defaultEncoding);

        bool slicedFormat = properties.getIcePropertyAsInt("Ice.Default.SlicedFormat") > 0;
        defaultFormat = slicedFormat ? FormatType.SlicedFormat : FormatType.CompactFormat;
    }

    public string defaultHost;
    public EndPoint defaultSourceAddress;
    public string defaultProtocol;
    public bool defaultCollocationOptimization;
    public Ice.EndpointSelectionType defaultEndpointSelection;
    public int defaultTimeout;
    public int defaultLocatorCacheTimeout;
    public int defaultInvocationTimeout;
    public bool defaultPreferSecure;
    public Ice.EncodingVersion defaultEncoding;
    public Ice.FormatType defaultFormat;

    public bool overrideTimeout;
    public int overrideTimeoutValue;
    public bool overrideConnectTimeout;
    public int overrideConnectTimeoutValue;
    public bool overrideCloseTimeout;
    public int overrideCloseTimeoutValue;
    public bool overrideCompress;
    public bool overrideCompressValue;
    public bool overrideSecure;
    public bool overrideSecureValue;
}
