//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    public ref struct PluginInitializationContext
    {
        private readonly Communicator _communicator;

        public ILogger Logger
        {
            get => _communicator.Logger;
            set =>_communicator.Logger = value;
        }

        public PluginInitializationContext(Communicator communicator) => _communicator = communicator;

        public void AddDispatchInterceptor(params DispatchInterceptor[] interceptor) =>
            _communicator.AddDispatchInterceptor(interceptor);

        public void AddInvocationInterceptor(params InvocationInterceptor[] interceptor) =>
            _communicator.AddInvocationInterceptor(interceptor);
    }
}
