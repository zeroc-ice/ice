// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;

public abstract class BasePluginFail implements Plugin {
    public BasePluginFail(Communicator communicator) {
        _communicator = communicator;
        _initialized = false;
        _destroyed = false;
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

    protected Communicator _communicator;
    protected boolean _initialized;
    protected boolean _destroyed;
    protected BasePluginFail _one;
    protected BasePluginFail _two;
    protected BasePluginFail _three;
}
