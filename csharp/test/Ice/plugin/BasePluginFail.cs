//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public abstract class BasePluginFail : Ice.IPlugin
{
    public BasePluginFail(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _initialized = false;
        _destroyed = false;
    }

    public bool isInitialized() => _initialized;

    public bool isDestroyed() => _destroyed;

    public abstract void Initialize();
    public abstract void Destroy();

    protected Ice.Communicator _communicator;
    protected bool _initialized;
    protected bool _destroyed;
    protected BasePluginFail? _one;
    protected BasePluginFail? _two;
    protected BasePluginFail? _three;
}
