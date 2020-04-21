//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using System;
using System.Collections.Generic;

namespace IceInternal
{
    public interface IEndpointFactory
    {
        void Initialize();
        EndpointType Type();
        string Transport();
        Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint);
        Endpoint Read(Ice.InputStream s);
        void Destroy();

        IEndpointFactory Clone(TransportInstance instance);
    }

    public abstract class EndpointFactoryWithUnderlying : IEndpointFactory
    {
        protected readonly TransportInstance Instance;
        private readonly EndpointType _type;
        private IEndpointFactory? _underlying;

        public void Initialize()
        {
            // Get the endpoint factory for the underlying type and clone it with our transport instance.
            IEndpointFactory? factory = Instance.GetEndpointFactory(_type);
            if (factory != null)
            {
                _underlying = factory.Clone(Instance);
                _underlying.Initialize();
            }
        }

        public EndpointType Type() => Instance.Type;

        public string Transport() => Instance!.Transport;

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint)
        {
            if (_underlying == null)
            {
                throw new InvalidOperationException(
                    $"cannot create a {Instance.Transport} endpoint without a factory for {_type}");
            }

            Endpoint underlyingEndpoint = _underlying.Create(endpointString, options, oaEndpoint);
            return CreateWithUnderlying(underlyingEndpoint, endpointString, options, oaEndpoint);
        }
        public Endpoint Read(Ice.InputStream istr)
        {
            if (_underlying == null)
            {
                throw new InvalidOperationException(
                    $"cannot create a {Instance.Transport} endpoint without a factory for {_type}");
            }
            return ReadWithUnderlying(_underlying.Read(istr), istr);
        }

        public void Destroy() => _underlying?.Destroy();

        public IEndpointFactory Clone(TransportInstance instance) => CloneWithUnderlying(instance, _type);

        public abstract IEndpointFactory CloneWithUnderlying(TransportInstance instance, EndpointType type);

        protected EndpointFactoryWithUnderlying(TransportInstance instance, EndpointType type)
        {
            Instance = instance;
            _type = type;
        }

        protected abstract Endpoint CreateWithUnderlying(Endpoint underlying,
            string endpointString, Dictionary<string, string?> options, bool oaEndpoint);
        protected abstract Endpoint ReadWithUnderlying(Endpoint underlying, Ice.InputStream istr);
    }
}
