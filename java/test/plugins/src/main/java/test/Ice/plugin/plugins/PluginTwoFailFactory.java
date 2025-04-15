// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginTwoFailFactory implements PluginFactory {
    @Override
    public String getPluginName() {
        return "Test";
    }

    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginTwoFail(communicator);
    }

    static class PluginTwoFail extends BasePluginFail {
        public PluginTwoFail(Communicator communicator) {
            super(communicator);
        }

        @Override
        public void initialize() {
            _one = (BasePluginFail) _communicator.getPluginManager().getPlugin("PluginOneFail");
            test(_one.isInitialized());
            _three = (BasePluginFail) _communicator.getPluginManager().getPlugin("PluginThreeFail");
            test(!_three.isInitialized());
            _initialized = true;
        }

        @Override
        public void destroy() {
            test(!_one.isDestroyed());
            // Not destroyed because initialize fails.
            test(!_three.isDestroyed());
            _destroyed = true;
        }

        @SuppressWarnings({"nofinalizer", "deprecation"})
        @Override
        protected void finalize() throws Throwable {
            try {
                if (!_initialized) {
                    System.out.println(getClass().getName() + " not initialized");
                }
                if (!_destroyed) {
                    System.out.println(getClass().getName() + " not destroyed");
                }
            } finally {
                super.finalize();
            }
        }
    }
}
