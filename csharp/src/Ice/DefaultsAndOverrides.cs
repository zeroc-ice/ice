//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Net;
using System.Text;

namespace IceInternal
{
    public sealed class DefaultsAndOverrides
    {
        internal DefaultsAndOverrides(Ice.Communicator communicator, Ice.Logger logger)
        {
            string? val;

            defaultProtocol = communicator.GetProperty("Ice.Default.Protocol") ?? "tcp";

            defaultHost = communicator.GetProperty("Ice.Default.Host");

            val = communicator.GetProperty("Ice.Default.SourceAddress");
            if (val != null)
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

            val = communicator.GetProperty("Ice.Override.Timeout");
            if (val != null)
            {
                overrideTimeout = true;
                overrideTimeoutValue = communicator.GetPropertyAsInt("Ice.Override.Timeout") ?? 0;
                if (overrideTimeoutValue < 1 && overrideTimeoutValue != -1)
                {
                    overrideTimeoutValue = -1;
                    StringBuilder msg = new StringBuilder("invalid value for Ice.Override.Timeout `");
                    msg.Append(communicator.GetProperty("Ice.Override.Timeout"));
                    msg.Append("': defaulting to -1");
                    logger.warning(msg.ToString());
                }
            }
            else
            {
                overrideTimeout = false;
                overrideTimeoutValue = -1;
            }

            val = communicator.GetProperty("Ice.Override.ConnectTimeout");
            if (val != null)
            {
                overrideConnectTimeout = true;
                overrideConnectTimeoutValue = communicator.GetPropertyAsInt("Ice.Override.ConnectTimeout") ?? -1;
                if (overrideConnectTimeoutValue < 1 && overrideConnectTimeoutValue != -1)
                {
                    overrideConnectTimeoutValue = -1;
                    StringBuilder msg = new StringBuilder("invalid value for Ice.Override.ConnectTimeout `");
                    msg.Append(communicator.GetProperty("Ice.Override.ConnectTimeout"));
                    msg.Append("': defaulting to -1");
                    logger.warning(msg.ToString());
                }
            }
            else
            {
                overrideConnectTimeout = false;
                overrideConnectTimeoutValue = -1;
            }

            val = communicator.GetProperty("Ice.Override.CloseTimeout");
            if (val != null)
            {
                overrideCloseTimeout = true;
                overrideCloseTimeoutValue = communicator.GetPropertyAsInt("Ice.Override.CloseTimeout") ?? -1;
                if (overrideCloseTimeoutValue < 1 && overrideCloseTimeoutValue != -1)
                {
                    overrideCloseTimeoutValue = -1;
                    StringBuilder msg = new StringBuilder("invalid value for Ice.Override.CloseTimeout `");
                    msg.Append(communicator.GetProperty("Ice.Override.CloseTimeout"));
                    msg.Append("': defaulting to -1");
                    logger.warning(msg.ToString());
                }
            }
            else
            {
                overrideCloseTimeout = false;
                overrideCloseTimeoutValue = -1;
            }

            val = communicator.GetProperty("Ice.Override.Compress");
            if (val != null)
            {
                overrideCompress = true;
                overrideCompressValue = communicator.GetPropertyAsInt("Ice.Override.Compress") > 0;
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

            val = communicator.GetProperty("Ice.Override.Secure");
            if (val != null)
            {
                overrideSecure = true;
                overrideSecureValue = communicator.GetPropertyAsInt("Ice.Override.Secure") > 0;
            }
            else
            {
                overrideSecure = false;
                overrideSecureValue = false;
            }

            defaultCollocationOptimization = (communicator.GetPropertyAsInt("Ice.Default.CollocationOptimized") ?? 1) > 0;

            val = communicator.GetProperty("Ice.Default.EndpointSelection") ?? "Random";
            if (val.Equals("Random"))
            {
                defaultEndpointSelection = Ice.EndpointSelectionType.Random;
            }
            else if (val.Equals("Ordered"))
            {
                defaultEndpointSelection = Ice.EndpointSelectionType.Ordered;
            }
            else
            {
                throw new ArgumentException($"illegal value `{val}'; expected `Random' or `Ordered'");
            }

            defaultTimeout = communicator.GetPropertyAsInt("Ice.Default.Timeout") ?? 60000;
            if (defaultTimeout < 1 && defaultTimeout != -1)
            {
                defaultTimeout = 60000;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Default.Timeout `");
                msg.Append(communicator.GetProperty("Ice.Default.Timeout"));
                msg.Append("': defaulting to 60000");
                logger.warning(msg.ToString());
            }

            defaultLocatorCacheTimeout = communicator.GetPropertyAsInt("Ice.Default.LocatorCacheTimeout") ?? -1;
            if (defaultLocatorCacheTimeout < -1)
            {
                defaultLocatorCacheTimeout = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Default.LocatorCacheTimeout `");
                msg.Append(communicator.GetProperty("Ice.Default.LocatorCacheTimeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }

            defaultInvocationTimeout = communicator.GetPropertyAsInt("Ice.Default.InvocationTimeout") ?? -1;
            if (defaultInvocationTimeout < 1 && defaultInvocationTimeout != -1 && defaultInvocationTimeout != -2)
            {
                defaultInvocationTimeout = -1;
                StringBuilder msg = new StringBuilder("invalid value for Ice.Default.InvocationTimeout `");
                msg.Append(communicator.GetProperty("Ice.Default.InvocationTimeout"));
                msg.Append("': defaulting to -1");
                logger.warning(msg.ToString());
            }

            defaultPreferSecure = communicator.GetPropertyAsInt("Ice.Default.PreferSecure") > 0;

            val = communicator.GetProperty("Ice.Default.EncodingVersion") ?? Ice.Util.encodingVersionToString(Ice.Util.currentEncoding);
            defaultEncoding = Ice.Util.stringToEncodingVersion(val);
            Protocol.checkSupportedEncoding(defaultEncoding);

            bool slicedFormat = communicator.GetPropertyAsInt("Ice.Default.SlicedFormat") > 0;
            defaultFormat = slicedFormat ? Ice.FormatType.SlicedFormat : Ice.FormatType.CompactFormat;
        }

        public string? defaultHost;
        public EndPoint? defaultSourceAddress;
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
