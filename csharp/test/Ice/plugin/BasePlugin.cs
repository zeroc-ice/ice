// Copyright (c) ZeroC, Inc.

public abstract class BasePlugin : Ice.Plugin
{
    public BasePlugin(Ice.Communicator communicator) => _communicator = communicator;

    public bool isInitialized() => _initialized;

    public bool isDestroyed() => _destroyed;

    protected static void test(bool b) => global::Test.TestHelper.test(b);

    public abstract void initialize();
    public abstract void destroy();

    protected Ice.Communicator _communicator;
    protected bool _initialized = false;
    protected bool _destroyed = false;
    protected BasePlugin _other = null;
}
