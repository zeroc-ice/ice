// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

public abstract class BasePlugin implements com.zeroc.Ice.Plugin {
    public BasePlugin(com.zeroc.Ice.Communicator communicator) {
        _communicator = communicator;
    }

    public boolean isInitialized() {
        return _initialized;
    }

    public boolean isDestroyed() {
        return _destroyed;
    }

    protected static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    protected com.zeroc.Ice.Communicator _communicator;
    protected boolean _initialized;
    protected boolean _destroyed;
    protected BasePlugin _other;
}
