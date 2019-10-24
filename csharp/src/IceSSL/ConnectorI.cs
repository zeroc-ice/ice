//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    internal sealed class ConnectorI : IceInternal.Connector
    {
        public IceInternal.Transceiver connect()
        {
            //
            // The plug-in may not be fully initialized.
            //
            if (!_instance.initialized())
            {
                Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                ex.reason = "IceSSL: plug-in is not initialized";
                throw ex;
            }

            return new TransceiverI(_instance, _delegate.connect(), _host, false);
        }

        public short type()
        {
            return _delegate.type();
        }

        //
        // Only for use by EndpointI.
        //
        internal ConnectorI(Instance instance, IceInternal.Connector del, string host)
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
        private IceInternal.Connector _delegate;
        private string _host;
    }
}
