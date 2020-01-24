//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace IceInternal
{
    public interface IEndpointFactory
    {
        void Initialize();
        short Type();
        string Protocol();
        Endpoint? Create(List<string> args, bool oaEndpoint);
        Endpoint? Read(Ice.InputStream s);
        void Destroy();

        IEndpointFactory Clone(ProtocolInstance instance);
    }

    public abstract class EndpointFactoryWithUnderlying : IEndpointFactory
    {
        public EndpointFactoryWithUnderlying(ProtocolInstance instance, short type)
        {
            Instance = instance;
            _type = type;
        }

        public void Initialize()
        {
            //
            // Get the endpoint factory for the underlying type and clone it with
            // our protocol instance.
            //
            IEndpointFactory? factory = Instance!.GetEndpointFactory(_type);
            if (factory != null)
            {
                _underlying = factory.Clone(Instance);
                _underlying.Initialize();
            }
        }

        public short Type() => Instance!.Type;

        public string Protocol() => Instance!.Protocol;

        public Endpoint? Create(List<string> args, bool oaEndpoint)
        {
            if (_underlying == null)
            {
                return null; // Can't create an endpoint without underlying factory.
            }
            return CreateWithUnderlying(_underlying.Create(args, oaEndpoint), args, oaEndpoint);
        }

        public Endpoint? Read(Ice.InputStream s)
        {
            if (_underlying == null)
            {
                return null; // Can't create an endpoint without underlying factory.
            }
            return ReadWithUnderlying(_underlying.Read(s), s);
        }

        public void Destroy()
        {
            if (_underlying != null)
            {
                _underlying.Destroy();
            }
            Instance = null;
        }

        public IEndpointFactory Clone(ProtocolInstance instance) => CloneWithUnderlying(instance, _type);

        public abstract IEndpointFactory CloneWithUnderlying(ProtocolInstance instance, short type);

        protected abstract Endpoint CreateWithUnderlying(Endpoint? underlying, List<string> args, bool oaEndpoint);
        protected abstract Endpoint ReadWithUnderlying(Endpoint? underlying, Ice.InputStream s);

        protected ProtocolInstance? Instance;

        private readonly short _type;
        private IEndpointFactory? _underlying;
    }
}
