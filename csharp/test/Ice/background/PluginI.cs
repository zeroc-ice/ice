//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Background
{
    internal class Plugin : IPlugin
    {
        internal Plugin(Communicator communicator) => _communicator = communicator;

        public void Initialize()
        {
            for (short s = 0; s < 100; ++s)
            {
                IEndpointFactory? factory = _communicator.IceFindEndpointFactory((EndpointType)s);
                if (factory != null)
                {
                    _communicator.IceAddEndpointFactory(new EndpointFactory(factory));
                }
            }
        }

        public void Destroy()
        {
        }

        private readonly Communicator _communicator;
    }
}
