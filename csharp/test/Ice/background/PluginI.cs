//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Background
{
    internal class Plugin : IPlugin
    {
        internal Plugin(Communicator communicator) => _communicator = communicator;

        public void Initialize()
        {
            for (short s = 0; s < 100; ++s)
            {
                var transport = (Transport)s;
                IEndpointFactory? factory = _communicator.IceFindEndpointFactory(transport);
                if (factory != null)
                {
                    var wrapper = new EndpointFactory(factory, transport);
                    _communicator.IceAddEndpointFactory(wrapper.Transport, wrapper.TransportName, wrapper);
                }
            }
        }

        public ValueTask DisposeAsync() => new ValueTask(Task.CompletedTask);

        private readonly Communicator _communicator;
    }
}
