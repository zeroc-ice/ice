//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal sealed class WSConnector : IConnector
    {
        public ITransceiver Connect() => new WSTransceiver(_instance, _delegate.Connect(), _host, _resource);

        public EndpointType Type() => _delegate.Type();

        internal WSConnector(TransportInstance instance, IConnector del, string host, string resource)
        {
            _instance = instance;
            _delegate = del;
            _host = host;
            _resource = resource;
        }

        public override bool Equals(object? obj)
        {
            if (!(obj is WSConnector))
            {
                return false;
            }

            if (this == obj)
            {
                return true;
            }

            var p = (WSConnector)obj;
            if (!_delegate.Equals(p._delegate))
            {
                return false;
            }

            if (!_resource.Equals(p._resource))
            {
                return false;
            }

            return true;
        }

        public override string ToString() => _delegate.ToString()!;

        public override int GetHashCode() => _delegate.GetHashCode();

        private readonly TransportInstance _instance;
        private readonly IConnector _delegate;
        private readonly string _host;
        private readonly string _resource;
    }
}
