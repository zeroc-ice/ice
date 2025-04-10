// Copyright (c) ZeroC, Inc.

package test.Ice.plugin.plugins;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Plugin;

public abstract class BasePlugin implements Plugin {
    public BasePlugin(Communicator communicator) {
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

    protected Communicator _communicator;
    protected boolean _initialized;
    protected boolean _destroyed;
    protected BasePlugin _other;
}
