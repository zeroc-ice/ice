// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    using System.Collections.Generic;

    public interface EndpointFactory
    {
        void initialize();
        short type();
        string protocol();
        EndpointI create(List<string> args, bool oaEndpoint);
        EndpointI read(Ice.InputStream s);
        void destroy();

        EndpointFactory clone(ProtocolInstance instance);
    }

    abstract public class EndpointFactoryWithUnderlying : EndpointFactory
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
            EndpointFactory factory = instance_.getEndpointFactory(_type);
            if(factory != null)
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

        public EndpointI create(List<string> args, bool oaEndpoint)
        {
            if(_underlying == null)
            {
                return null; // Can't create an endpoint without underlying factory.
            }
            return createWithUnderlying(_underlying.create(args, oaEndpoint), args, oaEndpoint);
        }

        public EndpointI read(Ice.InputStream s)
        {
            if(_underlying == null)
            {
                return null; // Can't create an endpoint without underlying factory.
            }
            return readWithUnderlying(_underlying.read(s), s);
        }

        public void destroy()
        {
            if(_underlying != null)
            {
                _underlying.destroy();
            }
            instance_ = null;
        }

        public EndpointFactory clone(ProtocolInstance instance)
        {
            return cloneWithUnderlying(instance, _type);
        }

        abstract public EndpointFactory cloneWithUnderlying(ProtocolInstance instance, short type);

        abstract protected EndpointI createWithUnderlying(EndpointI underlying, List<string> args, bool oaEndpoint);
        abstract protected EndpointI readWithUnderlying(EndpointI underlying, Ice.InputStream s);

        protected ProtocolInstance instance_;

        private readonly short _type;
        private EndpointFactory _underlying;
    }
}
