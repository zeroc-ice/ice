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

        val = properties.getIceProperty("Ice.Override.Compress");
        if (val.Length > 0)
        {
            overrideCompress = properties.getIcePropertyAsInt("Ice.Override.Compress") > 0;
            if (!BZip2.isLoaded(logger) && overrideCompress.Value)
            {
                Console.Error.WriteLine("warning: BZip2 library not found, Ice.Override.Compress ignored.");
                overrideCompress = null;
            }
        }
        else
        {
            overrideCompress = BZip2.isLoaded(logger) ? null : false;
        }

        val = properties.getIceProperty("Ice.Override.Secure");
        if (val.Length > 0)
        {
            overrideSecure = properties.getIcePropertyAsInt("Ice.Override.Secure") > 0;
        }
        else
        {
            overrideSecure = null;
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
            throw new ParseException($"illegal value '{val}' in property Ice.Default.EndpointSelection; expected 'Random' or 'Ordered'");
        }

        {
            int value = properties.getIcePropertyAsInt("Ice.Default.LocatorCacheTimeout");
            if (value < -1)
            {
                throw new InitializationException($"invalid value for Ice.Default.LocatorCacheTimeout: {value}");
            }
            defaultLocatorCacheTimeout = TimeSpan.FromSeconds(value);
        }

        {
            int value = properties.getIcePropertyAsInt("Ice.Default.InvocationTimeout");
            if (value < 1 && value != -1)
            {
                throw new InitializationException($"invalid value for Ice.Default.InvocationTimeout: {value}");
            }
            defaultInvocationTimeout = TimeSpan.FromMilliseconds(value);
        }

        defaultPreferSecure = properties.getIcePropertyAsInt("Ice.Default.PreferSecure") > 0;

        val = properties.getIceProperty("Ice.Default.EncodingVersion");
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
    public TimeSpan defaultLocatorCacheTimeout;
    public TimeSpan defaultInvocationTimeout;
    public bool defaultPreferSecure;
    public Ice.EncodingVersion defaultEncoding;
    public Ice.FormatType defaultFormat;

    public bool? overrideCompress;
    public bool? overrideSecure;
}
