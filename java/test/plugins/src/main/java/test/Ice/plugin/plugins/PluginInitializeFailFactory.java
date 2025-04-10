// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;
import com.zeroc.Ice.PluginFactory;

public class PluginInitializeFailFactory implements PluginFactory {
    @Override
    public Plugin create(
            Communicator communicator, String name, String[] args) {
        return new PluginInitializeFail();
    }

    static class PluginInitializeFail implements Plugin {
        @Override
        public void initialize() {
            throw new PluginInitializeFailException();
        }

        @Override
        public void destroy() {
            test(false);
        }

        private static void test(boolean b) {
            if (!b) {
                throw new RuntimeException();
            }
        }
    }
}
