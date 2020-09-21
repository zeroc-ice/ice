// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public abstract class BasePluginFail : IPlugin
    {
        public BasePluginFail(Communicator communicator)
        {
            _communicator = communicator;
            _initialized = false;
            _destroyed = false;
        }

        public bool isInitialized() => _initialized;

        public bool isDestroyed() => _destroyed;

        public abstract void Initialize(PluginInitializationContext context);
        public abstract ValueTask DisposeAsync();

        protected Communicator _communicator;
        protected bool _initialized;
        protected bool _destroyed;
        protected BasePluginFail? _one;
        protected BasePluginFail? _two;
        protected BasePluginFail? _three;
    }
}
