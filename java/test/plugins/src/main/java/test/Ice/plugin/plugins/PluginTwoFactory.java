// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginTwoFactory implements PluginFactory {
    @Override
    public String getPluginName() {
        return "Test";
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginTwo(communicator);
    }

    static class PluginTwo extends BasePlugin {
        public PluginTwo(Communicator communicator) {
            super(communicator);
        }

        @Override
        public void initialize() {
            _other = (BasePlugin) _communicator.getPluginManager().getPlugin("PluginOne");
            test(_other.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy() {
            _destroyed = true;
            test(!_other.isDestroyed());
        }
    }
}
