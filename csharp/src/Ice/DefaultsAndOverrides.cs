//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Net;

namespace IceInternal
{
    public sealed class DefaultsAndOverrides
    {
        internal DefaultsAndOverrides(Communicator communicator, ILogger logger)
        {
            string? val;

            DefaultTransport = communicator.GetProperty("Ice.Default.Transport") ?? "tcp";

            DefaultHost = communicator.GetProperty("Ice.Default.Host");

            val = communicator.GetProperty("Ice.Default.SourceAddress");
            if (val != null)
            {
                DefaultSourceAddress = Network.GetNumericAddress(val);
                if (DefaultSourceAddress == null)
                {
                    throw new InitializationException($"invalid IP address set for Ice.Default.SourceAddress: `{val}'");
                }
            }
            else
            {
                DefaultSourceAddress = null;
            }

            val = communicator.GetProperty("Ice.Override.Timeout");
            if (val != null)
            {
                OverrideTimeout = true;
                OverrideTimeoutValue = communicator.GetPropertyAsInt("Ice.Override.Timeout") ?? 0;
                if (OverrideTimeoutValue < 1 && OverrideTimeoutValue != -1)
                {
                    OverrideTimeoutValue = -1;
                    var msg = new System.Text.StringBuilder("invalid value for Ice.Override.Timeout `");
                    msg.Append(communicator.GetProperty("Ice.Override.Timeout"));
                    msg.Append("': defaulting to -1");
                    logger.Warning(msg.ToString());
                }
            }
            else
            {
                OverrideTimeout = false;
                OverrideTimeoutValue = -1;
            }

            val = communicator.GetProperty("Ice.Override.ConnectTimeout");
            if (val != null)
            {
                OverrideConnectTimeout = true;
                OverrideConnectTimeoutValue = communicator.GetPropertyAsInt("Ice.Override.ConnectTimeout") ?? -1;
                if (OverrideConnectTimeoutValue < 1 && OverrideConnectTimeoutValue != -1)
                {
                    OverrideConnectTimeoutValue = -1;
                    var msg = new System.Text.StringBuilder("invalid value for Ice.Override.ConnectTimeout `");
                    msg.Append(communicator.GetProperty("Ice.Override.ConnectTimeout"));
                    msg.Append("': defaulting to -1");
                    logger.Warning(msg.ToString());
                }
            }
            else
            {
                OverrideConnectTimeout = false;
                OverrideConnectTimeoutValue = -1;
            }

            val = communicator.GetProperty("Ice.Override.CloseTimeout");
            if (val != null)
            {
                OverrideCloseTimeout = true;
                OverrideCloseTimeoutValue = communicator.GetPropertyAsInt("Ice.Override.CloseTimeout") ?? -1;
                if (OverrideCloseTimeoutValue < 1 && OverrideCloseTimeoutValue != -1)
                {
                    OverrideCloseTimeoutValue = -1;
                    var msg = new System.Text.StringBuilder("invalid value for Ice.Override.CloseTimeout `");
                    msg.Append(communicator.GetProperty("Ice.Override.CloseTimeout"));
                    msg.Append("': defaulting to -1");
                    logger.Warning(msg.ToString());
                }
            }
            else
            {
                OverrideCloseTimeout = false;
                OverrideCloseTimeoutValue = -1;
            }

            val = communicator.GetProperty("Ice.Override.Compress");
            if (val != null)
            {
                OverrideCompress = true;
                OverrideCompressValue = communicator.GetPropertyAsInt("Ice.Override.Compress") > 0;
                if (!BZip2.Supported() && OverrideCompressValue)
                {
                    string lib = AssemblyUtil.IsWindows ? "bzip2.dll" : "libbz2.so.1";
                    Console.Error.WriteLine("warning: " + lib + " not found, Ice.Override.Compress ignored.");
                    OverrideCompressValue = false;
                }
            }
            else
            {
                OverrideCompress = !BZip2.Supported();
                OverrideCompressValue = false;
            }

            val = communicator.GetProperty("Ice.Override.Secure");
            if (val != null)
            {
                OverrideSecure = true;
                OverrideSecureValue = communicator.GetPropertyAsInt("Ice.Override.Secure") > 0;
            }
            else
            {
                OverrideSecure = false;
                OverrideSecureValue = false;
            }

            DefaultCollocationOptimization = (communicator.GetPropertyAsInt("Ice.Default.CollocationOptimized") ?? 1) > 0;

            val = communicator.GetProperty("Ice.Default.EndpointSelection") ?? "Random";
            if (val.Equals("Random"))
            {
                DefaultEndpointSelection = Ice.EndpointSelectionType.Random;
            }
            else if (val.Equals("Ordered"))
            {
                DefaultEndpointSelection = EndpointSelectionType.Ordered;
            }
            else
            {
                throw new ArgumentException($"illegal value `{val}'; expected `Random' or `Ordered'");
            }

            DefaultTimeout = communicator.GetPropertyAsInt("Ice.Default.Timeout") ?? 60000;
            if (DefaultTimeout < 1 && DefaultTimeout != -1)
            {
                DefaultTimeout = 60000;
                var msg = new System.Text.StringBuilder("invalid value for Ice.Default.Timeout `");
                msg.Append(communicator.GetProperty("Ice.Default.Timeout"));
                msg.Append("': defaulting to 60000");
                logger.Warning(msg.ToString());
            }

            DefaultLocatorCacheTimeout = communicator.GetPropertyAsInt("Ice.Default.LocatorCacheTimeout") ?? -1;
            if (DefaultLocatorCacheTimeout < -1)
            {
                DefaultLocatorCacheTimeout = -1;
                var msg = new System.Text.StringBuilder("invalid value for Ice.Default.LocatorCacheTimeout `");
                msg.Append(communicator.GetProperty("Ice.Default.LocatorCacheTimeout"));
                msg.Append("': defaulting to -1");
                logger.Warning(msg.ToString());
            }

            DefaultInvocationTimeout = communicator.GetPropertyAsInt("Ice.Default.InvocationTimeout") ?? -1;
            if (DefaultInvocationTimeout < 1 && DefaultInvocationTimeout != -1 && DefaultInvocationTimeout != -2)
            {
                DefaultInvocationTimeout = -1;
                var msg = new System.Text.StringBuilder("invalid value for Ice.Default.InvocationTimeout `");
                msg.Append(communicator.GetProperty("Ice.Default.InvocationTimeout"));
                msg.Append("': defaulting to -1");
                logger.Warning(msg.ToString());
            }

            DefaultPreferSecure = communicator.GetPropertyAsInt("Ice.Default.PreferSecure") > 0;

            val = communicator.GetProperty("Ice.Default.Encoding");
            if (val == null)
            {
                DefaultEncoding = Encoding.Latest;
            }
            else
            {
                DefaultEncoding = Encoding.Parse(val);
                DefaultEncoding.CheckSupported();
            }

            bool slicedFormat = communicator.GetPropertyAsInt("Ice.Default.SlicedFormat") > 0;
            DefaultFormat = slicedFormat ? Ice.FormatType.SlicedFormat : Ice.FormatType.CompactFormat;
        }

        public string? DefaultHost;
        public EndPoint? DefaultSourceAddress;
        public string DefaultTransport;
        public bool DefaultCollocationOptimization;
        public EndpointSelectionType DefaultEndpointSelection;
        public int DefaultTimeout;
        public int DefaultLocatorCacheTimeout;
        public int DefaultInvocationTimeout;
        public bool DefaultPreferSecure;
        public Encoding DefaultEncoding;
        public FormatType DefaultFormat;

        public bool OverrideTimeout;
        public int OverrideTimeoutValue;
        public bool OverrideConnectTimeout;
        public int OverrideConnectTimeoutValue;
        public bool OverrideCloseTimeout;
        public int OverrideCloseTimeoutValue;
        public bool OverrideCompress;
        public bool OverrideCompressValue;
        public bool OverrideSecure;
        public bool OverrideSecureValue;
    }

}
