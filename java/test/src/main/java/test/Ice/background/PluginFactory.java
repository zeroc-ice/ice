// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

public class PluginFactory implements com.zeroc.Ice.PluginFactory
{
    static public class PluginI implements com.zeroc.Ice.Plugin
    {
        public PluginI(com.zeroc.Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void initialize()
        {
            assert _configuration != null;
            com.zeroc.IceInternal.ProtocolPluginFacade facade =
                com.zeroc.IceInternal.Util.getProtocolPluginFacade(_communicator);
            for(short s = 0; s < 100; ++s)
            {
                com.zeroc.IceInternal.EndpointFactory factory = facade.getEndpointFactory(s);
                if(factory != null)
                {
                    facade.addEndpointFactory(new EndpointFactory(_configuration, factory));
                }
            }
        }

        @Override
        public void destroy()
        {
        }

        public void setConfiguration(Configuration configuration)
        {
            _configuration = configuration;
        }

        private final com.zeroc.Ice.Communicator _communicator;
        private Configuration _configuration;
    }

    @Override
    public com.zeroc.Ice.Plugin create(com.zeroc.Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
