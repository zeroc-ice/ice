// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Diagnostics;
using System.Diagnostics.CodeAnalysis;
using System.Threading;
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
        protected Communicator Communicator;
        protected bool Initialized;
        protected bool Destroyed;
        protected BasePlugin? Other;

        public BasePlugin(Communicator communicator) => Communicator = communicator;

        public bool IsInitialized() => Initialized;

        public bool IsDestroyed() => Destroyed;

        public abstract Task ActivateAsync(CancellationToken cancel);
        public virtual async ValueTask DisposeAsync()
        {
            GC.SuppressFinalize(this);
            if (Other != null)
            {
                await Other.DisposeAsync();
            }
        }
    }
}
