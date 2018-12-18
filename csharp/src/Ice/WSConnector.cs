// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

namespace IceInternal
{
    sealed class WSConnector : IceInternal.Connector
    {
        public IceInternal.Transceiver connect()
        {
            return new WSTransceiver(_instance, _delegate.connect(), _host, _resource);
        }

        public short type()
        {
            return _delegate.type();
        }

        internal WSConnector(ProtocolInstance instance, IceInternal.Connector del, string host, string resource)
        {
            _instance = instance;
            _delegate = del;
            _host = host;
            _resource = resource;
        }

        public override bool Equals(object obj)
        {
            if(!(obj is WSConnector))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            WSConnector p = (WSConnector)obj;
            if(!_delegate.Equals(p._delegate))
            {
                return false;
            }

            if(!_resource.Equals(p._resource))
            {
                return false;
            }

            return true;
        }

        public override string ToString()
        {
            return _delegate.ToString();
        }

        public override int GetHashCode()
        {
            return _delegate.GetHashCode();
        }

        private ProtocolInstance _instance;
        private Connector _delegate;
        private string _host;
        private string _resource;
    }
}
