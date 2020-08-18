//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

namespace ZeroC.Ice
{
    public sealed class PluginInitializationContext : IDisposable
    {
        private Communicator _communicator;
        private bool _disposed;

        public void Dispose() => _disposed = true;

        public ILogger Logger
        {
            get => _communicator.Logger;
            set
            {
                if (_disposed)
                {
                    throw new ObjectDisposedException("plug-in initialization context has been disposed");
                }
                _communicator.Logger = value;
            }
        }

        public PluginInitializationContext(Communicator communicator)
        {
            _communicator = communicator;
            _disposed = false;
        }

        public void AddDispatchInterceptor(params DispatchInterceptor[] interceptor)
        {
            if (_disposed)
            {
                throw new ObjectDisposedException("plug-in initialization context has been disposed");
            }
            _communicator.AddDispatchInterceptor(interceptor);
        }

        public void AddInvocationInterceptor(params InvocationInterceptor[] interceptor)
        {
            if (_disposed)
            {
                throw new ObjectDisposedException("plug-in initialization context has been disposed");
            }
            _communicator.AddInvocationInterceptor(interceptor);
        }
    }
}
