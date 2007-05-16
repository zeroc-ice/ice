// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

            val = properties.getProperty("Ice.Override.Compress");
            if(val.Length > 0)
            {
                overrideCompress = true;
                overrideCompressValue = properties.getPropertyAsInt("Ice.Override.Compress") != 0;
                if(!BasicStream.compressible() && overrideCompressValue)
                {
                    Console.Error.WriteLine("warning: libbz2 not installed, Ice.Override.Compress ignored");
                    overrideCompressValue = false;
                }
            }
            else
            {
                overrideCompress = !BasicStream.compressible();
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
                properties.getPropertyAsIntWithDefault("Ice.Default.CollocationOptimization", 1) > 0;

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
                ex.str = val;
                throw ex;
            }

            defaultLocatorCacheTimeout = properties.getPropertyAsIntWithDefault("Ice.Default.LocatorCacheTimeout", -1);

            defaultPreferSecure = properties.getPropertyAsIntWithDefault("Ice.Default.PreferSecure", 0) > 0;
        }
        
        public string defaultHost;
        public string defaultProtocol;
        public bool defaultCollocationOptimization;
        public Ice.EndpointSelectionType defaultEndpointSelection;
        public int defaultLocatorCacheTimeout;
        public bool defaultPreferSecure;
        
        public bool overrideTimeout;
        public int overrideTimeoutValue;
        public bool overrideConnectTimeout;
        public int overrideConnectTimeoutValue;
        public bool overrideCompress;
        public bool overrideCompressValue;
        public bool overrideSecure;
        public bool overrideSecureValue;
    }

}
