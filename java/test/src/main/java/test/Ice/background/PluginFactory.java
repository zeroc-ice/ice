// Copyright (c) ZeroC, Inc.

package test.Ice.background;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.Util;

public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    public static class PluginI implements Plugin {
        public PluginI(Communicator communicator) {
            _communicator = communicator;
        }

        @Override
        public void initialize() {
            var facade = Util.getProtocolPluginFacade(_communicator);
            for (short s = 0; s < 100; s++) {
                com.zeroc.Ice.EndpointFactory factory = facade.getEndpointFactory(s);
                if (factory != null) {
                    facade.addEndpointFactory(new EndpointFactory(_configuration, factory));
                }
            }
        }

        @Override
        public void destroy() {
        }

        public Configuration getConfiguration() {
            return _configuration;
        }

        private final Communicator _communicator;
        private Configuration _configuration = new Configuration();
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginI(communicator);
    }
}
