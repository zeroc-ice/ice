//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    internal sealed class ConnectorI : IceInternal.IConnector
    {
        public IceInternal.ITransceiver connect()
        {
            //
            // The plug-in may not be fully initialized.
            //
            if (!_instance.initialized())
            {
                throw new Ice.InitializationException("IceSSL: plug-in is not initialized");
            }

            return new Transceiver(_instance, _delegate.connect(), _host, false);
        }

        public short type()
        {
            return _delegate.type();
        }

        //
        // Only for use by EndpointI.
        //
        internal ConnectorI(Instance instance, IceInternal.IConnector del, string host)
        {
            _instance = instance;
            _delegate = del;
            _host = host;
        }

        public override bool Equals(object obj)
        {
            if (!(obj is ConnectorI))
            {
                return false;
            }

            if (this == obj)
            {
                return true;
            }

            ConnectorI p = (ConnectorI)obj;
            return _delegate.Equals(p._delegate);
        }

        public override string ToString()
        {
            return _delegate.ToString();
        }

        public override int GetHashCode()
        {
            return _delegate.GetHashCode();
        }

        private Instance _instance;
        private IceInternal.IConnector _delegate;
        private string _host;
    }
}
