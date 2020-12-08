// Copyright (c) ZeroC, Inc. All rights reserved.

using System;

namespace ZeroC.Ice
{
    /// <summary>The activation context is passed to plug-ins during activation.</summary>
    public sealed class PluginActivationContext : IDisposable
    {
        /// <summary>The communicator that is in the process of being activated.</summary>
        public Communicator Communicator { get; }

        private volatile bool _disposed;

        /// <summary>Gets or sets the communicator logger.</summary>
        public ILogger Logger
        {
            get => Communicator.Logger;
            set
            {
                if (_disposed)
                {
                    throw new ObjectDisposedException($"{typeof(PluginActivationContext).FullName}");
                }
                Communicator.Logger = value;
            }
        }

        /// <summary>Adds one or more dispatch interceptors to the communicator.</summary>
        /// <param name="interceptor">The dispatch interceptors to add.</param>
        public void AddDispatchInterceptor(params DispatchInterceptor[] interceptor)
        {
            if (_disposed)
            {
                throw new ObjectDisposedException($"{typeof(PluginActivationContext).FullName}");
            }
            Communicator.AddDispatchInterceptor(interceptor);
        }

        /// <summary>Adds one or more invocation interceptors to the communicator.</summary>
        /// <param name="interceptor">The invocation interceptors to add.</param>
        public void AddInvocationInterceptor(params InvocationInterceptor[] interceptor)
        {
            if (_disposed)
            {
                throw new ObjectDisposedException($"{typeof(PluginActivationContext).FullName}");
            }
            Communicator.AddInvocationInterceptor(interceptor);
        }

        public void Dispose() => _disposed = true;

        internal PluginActivationContext(Communicator communicator) => Communicator = communicator;
    }
}
