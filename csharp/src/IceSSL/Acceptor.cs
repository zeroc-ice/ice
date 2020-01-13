//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    internal class Acceptor : IceInternal.IAcceptor
    {
        public void close()
        {
            _delegate.close();
        }

        public IceInternal.Endpoint listen()
        {
            _endpoint = _endpoint.endpoint(_delegate.listen());
            return _endpoint;
        }

        public bool startAccept(IceInternal.AsyncCallback callback, object state)
        {
            //
            // The plug-in may not be fully initialized.
            //
            if (!_instance.initialized())
            {
                throw new Ice.InitializationException("IceSSL: plug-in is not initialized");
            }
            return _delegate.startAccept(callback, state);
        }

        public void finishAccept()
        {
            _delegate.finishAccept();
        }

        public IceInternal.ITransceiver accept()
        {
            return new Transceiver(_instance, _delegate.accept(), _adapterName, true);
        }

        public string protocol()
        {
            return _delegate.protocol();
        }

        public override string ToString()
        {
            return _delegate.ToString();
        }

        public string toDetailedString()
        {
            return _delegate.toDetailedString();
        }

        internal Acceptor(Endpoint endpoint, Instance instance, IceInternal.IAcceptor del, string adapterName)
        {
            _endpoint = endpoint;
            _delegate = del;
            _instance = instance;
            _adapterName = adapterName;

            //
            // .NET requires that a certificate be supplied.
            //
            var certs = instance.certs();
            if (certs == null || certs.Count == 0)
            {
                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: certificate required for server endpoint";
                throw ex;
            }
        }

        private Endpoint _endpoint;
        private IceInternal.IAcceptor _delegate;
        private Instance _instance;
        private string _adapterName;
    }
}
