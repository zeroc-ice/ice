//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
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
                    throw new InvalidConfigurationException(
                        $"invalid IP address set for Ice.Default.SourceAddress: `{val}'");
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
                if (!BZip2.IsLoaded && OverrideCompressValue)
                {
                    logger.Warning("compression not supported bzip2 library not found, Ice.Override.Compress ignored");
                    OverrideCompressValue = false;
                }
            }
            else
            {
                OverrideCompress = !BZip2.IsLoaded;
                OverrideCompressValue = false;
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
                throw new InvalidConfigurationException($"illegal value `{val}'; expected `Random' or `Ordered'");
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
            if (DefaultInvocationTimeout < 1 && DefaultInvocationTimeout != -1)
            {
                DefaultInvocationTimeout = -1;
                var msg = new System.Text.StringBuilder("invalid value for Ice.Default.InvocationTimeout `");
                msg.Append(communicator.GetProperty("Ice.Default.InvocationTimeout"));
                msg.Append("': defaulting to -1");
                logger.Warning(msg.ToString());
            }

            // TODO: switch to 0 default
            DefaultPreferNonSecure = (communicator.GetPropertyAsInt("Ice.Default.PreferNonSecure") ?? 1) > 0;

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
            DefaultFormat = slicedFormat ? Ice.FormatType.Sliced : Ice.FormatType.Compact;
        }

        public string? DefaultHost;
        public IPAddress? DefaultSourceAddress;
        public string DefaultTransport;
        public bool DefaultCollocationOptimization;
        public EndpointSelectionType DefaultEndpointSelection;
        public int DefaultTimeout;
        public int DefaultLocatorCacheTimeout;
        public int DefaultInvocationTimeout;
        public bool DefaultPreferNonSecure;
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
    }

}
