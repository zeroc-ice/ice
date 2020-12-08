// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public abstract class BasePluginFail : IPlugin
    {
        protected Communicator _communicator;
        protected bool _destroyed;
        protected bool _initialized;
        protected BasePluginFail? _one;
        protected BasePluginFail? _two;
        protected BasePluginFail? _three;

        public BasePluginFail(Communicator communicator)
        {
            _communicator = communicator;
            _initialized = false;
            _destroyed = false;
        }

        public bool isInitialized() => _initialized;

        public bool isDestroyed() => _destroyed;

        public abstract Task ActivateAsync(CancellationToken cancel);
        public abstract ValueTask DisposeAsync();
    }
}
