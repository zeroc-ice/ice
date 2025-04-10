// Copyright (c) ZeroC, Inc.

package test.Ice.classLoader;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginFactoryI implements PluginFactory {
    static class PluginI implements Plugin {
        @Override
        public void initialize() {}

        @Override
        public void destroy() {}
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginI();
    }
}
