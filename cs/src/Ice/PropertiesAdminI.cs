// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;

namespace IceInternal
{
    sealed class PropertiesAdminI : Ice.PropertiesAdminDisp_
    {
        public PropertiesAdminI(Ice.Properties properties)
        {
            _properties = properties;
        }

        public override string
        getProperty(string name, Ice.Current current)
        {
            return _properties.getProperty(name);
        }
        
        public override Dictionary<string, string>
        getPropertiesForPrefix(string name, Ice.Current current)
        {
            return _properties.getPropertiesForPrefix(name);
        }
        
        private Ice.Properties _properties;
    }
}
