// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginThreeFailFactory implements PluginFactory {
    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginThreeFail(communicator);
    }

    public class PluginThreeFail extends BasePluginFail {
        public PluginThreeFail(Communicator communicator) {
            super(communicator);
        }

        @Override
        public void initialize() {
            throw new PluginInitializeFailException();
        }

        @Override
        public void destroy() {
            test(false);
        }

        @SuppressWarnings("nofinalizer")
        @Override
        protected void finalize() throws Throwable {
            try {
                if (_initialized) {
                    System.out.println(getClass().getName() + " was initialized");
                }
                if (_destroyed) {
                    System.out.println(getClass().getName() + " was destroyed");
                }
            } finally {
                super.finalize();
            }
        }
    }
}
