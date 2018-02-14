// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.background;

public class PluginFactory implements Ice.PluginFactory
{
    static public class PluginI implements Ice.Plugin
    {
        public
        PluginI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }

        @Override
        public void
        initialize()
        {
            IceInternal.ProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(_communicator);
            for(short s = 0; s < 100; ++s)
            {
                IceInternal.EndpointFactory factory = facade.getEndpointFactory(s);
                if(factory != null)
                {
                    facade.addEndpointFactory(new EndpointFactory(_configuration, factory));
                }
            }
        }

        @Override
        public void
        destroy()
        {
        }

        public Configuration
        getConfiguration()
        {
            return _configuration;
        }

        private final Ice.Communicator _communicator;
        private Configuration _configuration = new Configuration();
    }

    @Override
    public Ice.Plugin
    create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
