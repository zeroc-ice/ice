//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    internal sealed class ConnectorI : IceInternal.IConnector
    {
        public IceInternal.ITransceiver Connect()
        {
            //
            // The plug-in may not be fully initialized.
            //
            if (!_instance.Initialized())
            {
                throw new System.InvalidOperationException("IceSSL: plug-in is not initialized");
            }

            return new Transceiver(_instance, _delegate.Connect(), _host, false);
        }

        public Ice.EndpointType Type() => _delegate.Type();

        //
        // Only for use by EndpointI.
        //
        internal ConnectorI(Instance instance, IceInternal.IConnector del, string host)
        {
            _instance = instance;
            _delegate = del;
            _host = host;
        }

        public override bool Equals(object? obj)
        {
            if (!(obj is ConnectorI))
            {
                return false;
            }

            if (this == obj)
            {
                return true;
            }

            var p = (ConnectorI)obj;
            return _delegate.Equals(p._delegate);
        }

        public override string ToString() => _delegate.ToString()!;

        public override int GetHashCode() => _delegate.GetHashCode();

        private readonly Instance _instance;
        private readonly IceInternal.IConnector _delegate;
        private readonly string _host;
    }
}
