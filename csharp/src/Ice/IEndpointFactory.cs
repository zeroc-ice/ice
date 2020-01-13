//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceInternal
{
    using System.Collections.Generic;

    public interface IEndpointFactory
    {
        void initialize();
        short type();
        string protocol();
        Endpoint create(List<string> args, bool oaEndpoint);
        Endpoint read(Ice.InputStream s);
        void destroy();

        IEndpointFactory clone(ProtocolInstance instance);
    }

    public abstract class EndpointFactoryWithUnderlying : IEndpointFactory
    {
        public EndpointFactoryWithUnderlying(ProtocolInstance instance, short type)
        {
            instance_ = instance;
            _type = type;
        }

        public void initialize()
        {
            //
            // Get the endpoint factory for the underlying type and clone it with
            // our protocol instance.
            //
            IEndpointFactory factory = instance_.getEndpointFactory(_type);
            if (factory != null)
            {
                _underlying = factory.clone(instance_);
                _underlying.initialize();
            }
        }

        public short type()
        {
            return instance_.type();
        }

        public string protocol()
        {
            return instance_.protocol();
        }

        public Endpoint create(List<string> args, bool oaEndpoint)
        {
            if (_underlying == null)
            {
                return null; // Can't create an endpoint without underlying factory.
            }
            return createWithUnderlying(_underlying.create(args, oaEndpoint), args, oaEndpoint);
        }

        public Endpoint read(Ice.InputStream s)
        {
            if (_underlying == null)
            {
                return null; // Can't create an endpoint without underlying factory.
            }
            return readWithUnderlying(_underlying.read(s), s);
        }

        public void destroy()
        {
            if (_underlying != null)
            {
                _underlying.destroy();
            }
            instance_ = null;
        }

        public IEndpointFactory clone(ProtocolInstance instance)
        {
            return cloneWithUnderlying(instance, _type);
        }

        public abstract IEndpointFactory cloneWithUnderlying(ProtocolInstance instance, short type);

        protected abstract Endpoint createWithUnderlying(Endpoint underlying, List<string> args, bool oaEndpoint);
        protected abstract Endpoint readWithUnderlying(Endpoint underlying, Ice.InputStream s);

        protected ProtocolInstance instance_;

        private readonly short _type;
        private IEndpointFactory _underlying;
    }
}
