// Copyright (c) ZeroC, Inc.

namespace Ice.plugin;

public abstract class BasePlugin : Ice.Plugin
{
    protected BasePlugin(Ice.Communicator communicator) => _communicator = communicator;

    public bool isInitialized() => _initialized;

    public bool isDestroyed() => _destroyed;

    protected static void test(bool b) => global::Test.TestHelper.test(b);

    public abstract void initialize();

    public abstract void destroy();

    protected Ice.Communicator _communicator;
    protected bool _initialized;
    protected bool _destroyed;
    protected BasePlugin _other;
}
