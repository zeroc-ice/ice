//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public abstract class BasePlugin : Ice.IPlugin
{
    public BasePlugin(Ice.Communicator communicator) => _communicator = communicator;

    public bool isInitialized() => _initialized;

    public bool isDestroyed() => _destroyed;

    protected static void test(bool b)
    {
        if (!b)
        {
            System.Diagnostics.Debug.Assert(false);
            throw new System.Exception();
        }
    }

    public abstract void Initialize();
    public abstract void Destroy();

    protected Ice.Communicator _communicator;
    protected bool _initialized = false;
    protected bool _destroyed = false;
    protected BasePlugin _other = null;
}
