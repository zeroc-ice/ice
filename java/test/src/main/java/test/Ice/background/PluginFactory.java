// Copyright (c) ZeroC, Inc.

package test.Ice.background;

public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    public static class PluginI implements com.zeroc.Ice.Plugin {
        public PluginI(com.zeroc.Ice.Communicator communicator) {
            _communicator = communicator;
        }

        @Override
        public void initialize() {
            var facade = com.zeroc.Ice.Util.getProtocolPluginFacade(_communicator);
            for (short s = 0; s < 100; ++s) {
                com.zeroc.Ice.EndpointFactory factory = facade.getEndpointFactory(s);
                if (factory != null) {
                    facade.addEndpointFactory(new EndpointFactory(_configuration, factory));
                }
            }
        }

        @Override
        public void destroy() {}

        public Configuration getConfiguration() {
            return _configuration;
        }

        private final com.zeroc.Ice.Communicator _communicator;
        private Configuration _configuration = new Configuration();
    }

    @Override
    public com.zeroc.Ice.Plugin create(
            com.zeroc.Ice.Communicator communicator, String name, String[] args) {
        return new PluginI(communicator);
    }
}
