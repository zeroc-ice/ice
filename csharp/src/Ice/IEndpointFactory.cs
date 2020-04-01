//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System.Collections.Generic;

namespace IceInternal
{
    public interface IEndpointFactory
    {
        void Initialize();
        EndpointType Type();
        string Transport();
        Endpoint? Create(List<string> args, bool oaEndpoint);
        Endpoint? Read(Ice.InputStream s);
        void Destroy();

        IEndpointFactory Clone(TransportInstance instance);
    }

    public abstract class EndpointFactoryWithUnderlying : IEndpointFactory
    {
        public EndpointFactoryWithUnderlying(TransportInstance instance, EndpointType type)
        {
            Instance = instance;
            _type = type;
        }

        public void Initialize()
        {
            //
            // Get the endpoint factory for the underlying type and clone it with
            // our transport instance.
            //
            IEndpointFactory? factory = Instance!.GetEndpointFactory(_type);
            if (factory != null)
            {
                _underlying = factory.Clone(Instance);
                _underlying.Initialize();
            }
        }

        public EndpointType Type() => Instance!.Type;

        public string Transport() => Instance!.Transport;

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

        public IEndpointFactory Clone(TransportInstance instance) => CloneWithUnderlying(instance, _type);

        public abstract IEndpointFactory CloneWithUnderlying(TransportInstance instance, EndpointType type);

        protected abstract Endpoint CreateWithUnderlying(Endpoint? underlying, List<string> args, bool oaEndpoint);
        protected abstract Endpoint ReadWithUnderlying(Endpoint? underlying, Ice.InputStream s);

        protected TransportInstance? Instance;

        private readonly EndpointType _type;
        private IEndpointFactory? _underlying;
    }
}
