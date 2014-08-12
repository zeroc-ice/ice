// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{
    sealed class WSConnectorI : IceInternal.Connector
    {
        public IceInternal.Transceiver connect()
        {
            return new WSTransceiverI(_instance, _delegate.connect(), _host, _port, _resource);
        }

        public short type()
        {
            return _delegate.type();
        }

        internal WSConnectorI(ProtocolInstance instance, IceInternal.Connector del, string host, int port, string resource)
        {
            _instance = instance;
            _delegate = del;
            _host = host;
            _port = port;
            _resource = resource;
        }

        public override bool Equals(object obj)
        {
            if(!(obj is WSConnectorI))
            {
                return false;
            }

            if(this == obj)
            {
                return true;
            }

            WSConnectorI p = (WSConnectorI)obj;
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
        private IceInternal.Connector _delegate;
        private string _host;
        private int _port;
        private string _resource;
    }
}
