// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

namespace IceInternal
{

    public sealed class DefaultsAndOverrides
    {
        internal DefaultsAndOverrides(Ice.Properties properties)
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

            val = properties.getProperty("Ice.Override.Timeout");
            if(val.Length > 0)
            {
                overrideTimeout = true;
                overrideTimeoutValue = properties.getPropertyAsInt("Ice.Override.Timeout");
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
            }
            else
            {
                overrideCloseTimeout = false;
                overrideCloseTimeoutValue = -1;
            }

#if COMPACT
            overrideCompress = false;
            overrideCompressValue = false;
#else
            val = properties.getProperty("Ice.Override.Compress");
            if(val.Length > 0)
            {
                overrideCompress = true;
                overrideCompressValue = properties.getPropertyAsInt("Ice.Override.Compress") != 0;
                if(!BasicStream.compressible() && overrideCompressValue)
                {
                    string lib = AssemblyUtil.runtime_ == AssemblyUtil.Runtime.Mono ? "bzip2 library" : "bzip2.dll";
                    Console.Error.WriteLine("warning: " + lib + " not found, Ice.Override.Compress ignored.");
                    overrideCompressValue = false;
                }
            }
            else
            {
                overrideCompress = !BasicStream.compressible();
                overrideCompressValue = false;
            }
#endif

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

            defaultLocatorCacheTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);

            defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;

            val = properties.getPropertyWithDefault("Ice.Default.EncodingVersion",
                                                    Ice.Util.encodingVersionToString(Ice.Util.currentEncoding));
            defaultEncoding = Ice.Util.stringToEncodingVersion(val);
            Protocol.checkSupportedEncoding(defaultEncoding);

            bool slicedFormat = properties.getPropertyAsIntWithDefault("Ice.Default.SlicedFormat", 0) > 0;
            defaultFormat = slicedFormat ? Ice.FormatType.SlicedFormat : Ice.FormatType.CompactFormat;
        }

        public string defaultHost;
        public string defaultProtocol;
        public bool defaultCollocationOptimization;
        public Ice.EndpointSelectionType defaultEndpointSelection;
        public int defaultLocatorCacheTimeout;
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
