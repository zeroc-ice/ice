//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Plugin
{
    public static class TestHelper
    {
        public static void Assert([DoesNotReturnIf(false)] bool b)
        {
            if (!b)
            {
                Debug.Assert(false);
                throw new Exception();
            }
        }
    }
    public abstract class BasePlugin : IPlugin
    {
        public BasePlugin(Communicator communicator) => _communicator = communicator;

        public bool isInitialized() => _initialized;

        public bool isDestroyed() => _destroyed;

        public abstract void Initialize();
        public virtual async ValueTask DisposeAsync()
        {
            if (_other != null)
            {
                await _other.DisposeAsync();
            }
        }

        protected Communicator _communicator;
        protected bool _initialized = false;
        protected bool _destroyed = false;
        protected BasePlugin? _other = null;
    }
}
