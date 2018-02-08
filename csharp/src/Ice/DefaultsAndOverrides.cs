// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Net;
using System.Text;

namespace IceInternal
{
    public sealed class DefaultsAndOverrides
    {
        internal DefaultsAndOverrides(Ice.Properties properties, Ice.Logger logger)
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
                defaultHost = null;
            }

            val = properties.getProperty("Ice.Default.SourceAddress");
            if(val.Length > 0)
            {
                defaultSourceAddress = Network.getNumericAddress(val);
                if(defaultSourceAddress == null)
                {
                    throw new Ice.InitializationException("invalid IP address set for Ice.Default.SourceAddress: `" +
                                                          val + "'");
                }
            }
            else
            {
                defaultSourceAddress = null;
            }

            val = properties.getProperty("Ice.Override.Timeout");
            if(val.Length > 0)
            {
                overrideTimeout = true;
                overrideTimeoutValue = properties.getPropertyAsInt("Ice.Override.Timeout");
                if(overrideTimeoutValue < 1 && overrideTimeoutValue != -1)
                {
                    overrideTimeoutValue = -1;
                    StringBuilder msg = new StringBuilder("invalid value for Ice.Override.Timeout `");
                    msg.Append(properties.getProperty("Ice.Override.Timeout"));
                    msg.Append("': defaulting to -1");
                    logger.warning(msg.ToString());
                }
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
                if(overrideConnectTimeoutValue < 1 && overrideConnectTimeoutValue != -1)
                {
                    overrideConnectTimeoutValue = -1;
                    StringBuilder msg = new StringBuilder("invalid value for Ice.Override.ConnectTimeout `");
                    msg.Append(properties.getProperty("Ice.Override.ConnectTimeout"));
                    msg.Append("': defaulting to -1");
                    logger.warning(msg.ToString());
                }
            }
            else
            {
                overrideConnectTimeout = false;
                overrideConnectTimeoutValue = -1;
            }

            val = properties.getProperty("Ice.Override.CloseTimeout");
            if(val.Length > 0)
            {
                overrideCloseTimeout = true;
                overrideCloseTimeoutValue = properties.getPropertyAsInt("Ice.Override.CloseTimeout");
                if(overrideCloseTimeoutValue < 1 && overrideCloseTimeoutValue != -1)
                {
                    overrideCloseTimeoutValue = -1;
                    StringBuilder msg = new StringBuilder("invalid value for Ice.Override.CloseTimeout `");
                    msg.Append(properties.getProperty("Ice.Override.CloseTimeout"));
                    msg.Append("': defaulting to -1");
                    logger.warning(msg.ToString());
                }
            }
            else
            {
                overrideCloseTimeout = false;
                overrideCloseTimeoutValue = -1;
            }

            val = properties.getProperty("Ice.Override.Compress");
            if(val.Length > 0)
            {
                overrideCompress = true;
                overrideCompressValue = properties.getPropertyAsInt("Ice.Override.Compress") > 0;
                if(!BZip2.supported() && overrideCompressValue)
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

            val = properties.getProperty("Ice.Override.Secure");
            if(val.Length > 0)
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

            val = properties.getPropertyWithDefault("Ice.Default.EndpointSelection", "Random");
            if(val.Equals("Random"))
            {
                defaultEndpointSelection = Ice.EndpointSelectionType.Random;
            }
            else if(val.Equals("Ordered"))
            {
                defaultEndpointSelection = Ice.EndpointSelectionType.Ordered;
            }
            else
            {
                Ice.EndpointSelectionTypeParseException ex = new Ice.EndpointSelectionTypeParseException();
                ex.str = "illegal value `" + val + "'; expected `Random' or `Ordered'";
                throw ex;
            }

            defaultTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.Timeout", 60000);
            if(defaultTimeout < 1 && defaultTimeout != -1)
            {
                defaultTimeout = 60000;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Default.Timeout `");
                msg.Append(properties.getProperty("Ice.Default.Timeout"));
                msg.Append("': defaulting to 60000");
                logger.warning(msg.ToString());
            }

            defaultLocatorCacheTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);
            if(defaultLocatorCacheTimeout < -1)
            {
                defaultLocatorCacheTimeout = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Default.LocatorCacheTimeout `");
                msg.Append(properties.getProperty("Ice.Default.LocatorCacheTimeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }

            defaultInvocationTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.InvocationTimeout", -1);
            if(defaultInvocationTimeout < 1 && defaultInvocationTimeout != -1 && defaultInvocationTimeout != -2)
            {
                defaultInvocationTimeout = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Default.InvocationTimeout `");
                msg.Append(properties.getProperty("Ice.Default.InvocationTimeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }

            defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

            val = properties.getPropertyWithDefault("Ice.Default.EncodingVersion",
                                                    Ice.Util.encodingVersionToString(Ice.Util.currentEncoding));
            defaultEncoding = Ice.Util.stringToEncodingVersion(val);
            Protocol.checkSupportedEncoding(defaultEncoding);

            bool slicedFormat = properties.getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
            defaultFormat = slicedFormat ? Ice.FormatType.SlicedFormat : Ice.FormatType.CompactFormat;
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

}
