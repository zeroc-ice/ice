// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginThreeFactory implements PluginFactory {
    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginThree(communicator);
    }

    static class PluginThree extends BasePlugin {
        public PluginThree(Communicator communicator) {
            super(communicator);
        }

        @Override
        public void initialize() {
            _other = (BasePlugin) _communicator.getPluginManager().getPlugin("PluginTwo");
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
