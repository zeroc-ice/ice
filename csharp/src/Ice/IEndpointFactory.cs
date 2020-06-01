//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;

namespace ZeroC.Ice
{
    public interface IEndpointFactory
    {
        void Initialize();
        EndpointType Type { get; }
        string Transport { get; }
        Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint);
        Endpoint Read(InputStream s);
        void Destroy();

        IEndpointFactory Clone(string transport, EndpointType type);
    }

    public abstract class EndpointFactoryWithUnderlying : IEndpointFactory
    {
        public EndpointType Type { get; }
        public string Transport { get; }

        protected readonly Communicator Communicator;
        private readonly EndpointType _underlyingType;
        private IEndpointFactory? _underlying;

        public void Initialize()
        {
            // Get the endpoint factory for the underlying type and clone it with our transport instance.
            IEndpointFactory? factory = Communicator.IceFindEndpointFactory(_underlyingType);
            if (factory != null)
            {
                _underlying = factory.Clone(Transport, Type);
                _underlying.Initialize();
            }
        }

        public Endpoint Create(string endpointString, Dictionary<string, string?> options, bool oaEndpoint)
        {
            if (_underlying == null)
            {
                throw new InvalidOperationException(
                    $"cannot create a {Transport} endpoint without a factory for {_underlyingType}");
            }

            Endpoint underlyingEndpoint = _underlying.Create(endpointString, options, oaEndpoint);
            return CreateWithUnderlying(underlyingEndpoint, endpointString, options, oaEndpoint);
        }
        public Endpoint Read(InputStream istr)
        {
            if (_underlying == null)
            {
                throw new InvalidOperationException(
                    $"cannot create a {Transport} endpoint without a factory for {_underlyingType}");
            }
            return ReadWithUnderlying(_underlying.Read(istr), istr);
        }

        public void Destroy() => _underlying?.Destroy();

        public IEndpointFactory Clone(string transport, EndpointType type) =>
            CloneWithUnderlying(transport, type, _underlyingType);

        public abstract IEndpointFactory CloneWithUnderlying(
            string transport, EndpointType type, EndpointType underlyingType);

        protected EndpointFactoryWithUnderlying(Communicator communicator, string transport, EndpointType type,
            EndpointType underlyingType)
        {
            Communicator = communicator;
            Transport = transport;
            Type = type;
            _underlyingType = underlyingType;
        }

        protected abstract Endpoint CreateWithUnderlying(Endpoint underlying,
            string endpointString, Dictionary<string, string?> options, bool oaEndpoint);
        protected abstract Endpoint ReadWithUnderlying(Endpoint underlying, InputStream istr);
    }
}
