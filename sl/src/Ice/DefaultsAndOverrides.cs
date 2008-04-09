// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
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
        }
        
        public string defaultHost;
        public string defaultProtocol;
        
        public bool overrideSecure;
        public bool overrideSecureValue;
    }

}
