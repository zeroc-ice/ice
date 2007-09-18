// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
        
        public override string
        getPropertyWithDefault(string name, string dflt, Ice.Current current)
        {
            return _properties.getPropertyWithDefault(name, dflt);
        }
        
        public override int
        getPropertyAsInt(string name, Ice.Current current)
        {
            return _properties.getPropertyAsInt(name);
        }
        
        public override int
        getPropertyAsIntWithDefault(string name, int dflt, Ice.Current current)
        {
            return _properties.getPropertyAsIntWithDefault(name, dflt);
        }
        
        public override Ice.PropertyDict
        getPropertiesForPrefix(string name, Ice.Current current)
        {
            return _properties.getPropertiesForPrefix(name);
        }
        
        private Ice.Properties _properties;
    }
}
