//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    /// <summary>The plug-in initialization context is passed to plug-ins during initialization allowing plug-ins to
    /// set the Communicator's logger, or add dispatch and invocation interceptors.</summary>
    public ref struct PluginInitializationContext
    {
        private readonly Communicator _communicator;

        /// <summary>Gets or sets the communicator logger.</summary>
        public ILogger Logger
        {
            get => _communicator.Logger;
            set => _communicator.Logger = value;
        }

        internal PluginInitializationContext(Communicator communicator) => _communicator = communicator;

        /// <summary>Adds one or more dispatch interceptors to the communicator.</summary>
        /// <param name="interceptor">The dispatch interceptors to add.</param>
        public void AddDispatchInterceptor(params DispatchInterceptor[] interceptor) =>
            _communicator.AddDispatchInterceptor(interceptor);

        /// <summary>Adds one or more invocation interceptors to the communicator.</summary>
        /// <param name="interceptor">The invocation interceptors to add.</param>
        public void AddInvocationInterceptor(params InvocationInterceptor[] interceptor) =>
            _communicator.AddInvocationInterceptor(interceptor);
    }
}
