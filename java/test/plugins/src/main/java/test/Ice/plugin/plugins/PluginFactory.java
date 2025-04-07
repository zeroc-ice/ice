// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;

public class PluginFactory implements com.zeroc.Ice.PluginFactory {
    @Override
    public com.zeroc.Ice.Plugin create(
            Communicator communicator, String name, String[] args) {
        return new Plugin(args);
    }

    static class Plugin implements com.zeroc.Ice.Plugin {
        public Plugin(String[] args) {
            _args = args;
        }

        @Override
        public void initialize() {
            _initialized = true;
            test(_args.length == 3);
            test("C:\\Program Files\\".equals(_args[0]));
            test("--DatabasePath".equals(_args[1]));
            test("C:\\Program Files\\Application\\db".equals(_args[2]));
        }

        @Override
        public void destroy() {
            _destroyed = true;
        }

        @SuppressWarnings("deprecation")
        @Override
        protected void finalize() throws Throwable {
            try {
                if (!_initialized) {
                    System.out.println("test.Ice.plugin.plugins.Plugin not initialized");
                }
                if (!_destroyed) {
                    System.out.println("test.Ice.plugin.plugins.Plugin not destroyed");
                }
            } finally {
                super.finalize();
            }
        }

        private static void test(boolean b) {
            if (!b) {
                throw new RuntimeException();
            }
        }

        private String[] _args;
        private boolean _initialized;
        private boolean _destroyed;
    }
}
