// Copyright (c) ZeroC, Inc.

public abstract class BasePluginFail : Ice.Plugin
{
    public BasePluginFail(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _initialized = false;
        _destroyed = false;
    }

    public bool isInitialized() => _initialized;

    public bool isDestroyed() => _destroyed;

    protected static void test(bool b) => global::Test.TestHelper.test(b);

    public abstract void initialize();
    public abstract void destroy();

    protected Ice.Communicator _communicator;
    protected bool _initialized;
    protected bool _destroyed;
    protected BasePluginFail _one;
    protected BasePluginFail _two;
    protected BasePluginFail _three;
}
