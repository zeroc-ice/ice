// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class PluginFactory implements Ice.PluginFactory
{
    static class PluginI implements Ice.Plugin
    {
        public
        PluginI(Ice.Communicator communicator)
        {
            _communicator = communicator;
        }
        
        public void
        initialize()
        {
            IceInternal.ProtocolPluginFacade facade = Ice.Util.getProtocolPluginFacade(_communicator);
            for(short s = 0; s < 100; ++s)
            {
                IceInternal.EndpointFactory factory = facade.getEndpointFactory(s);
                if(factory != null)
                {
                    facade.addEndpointFactory(new EndpointFactory(factory));
                }
            }
        }

        public void
        destroy()
        {
        }

        private final Ice.Communicator _communicator;
    }

    public Ice.Plugin
    create(Ice.Communicator communicator, String name, String[] args)
    {
        return new PluginI(communicator);
    }
}
