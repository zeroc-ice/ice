// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    class AcceptorI : IceInternal.Acceptor
    {
        public void close()
        {
            _delegate.close();
        }

        public IceInternal.EndpointI listen()
        {
            _endpoint = _endpoint.endpoint(_delegate.listen());
            return _endpoint;
        }

        public bool startAccept(IceInternal.AsyncCallback callback, object state)
        {
            //
            // The plug-in may not be fully initialized.
            //
            if(!_instance.initialized())
            {
                Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                ex.reason = "IceSSL: plug-in is not initialized";
                throw ex;
            }
            return _delegate.startAccept(callback, state);
        }

        public void finishAccept()
        {
            _delegate.finishAccept();
        }

        public IceInternal.Transceiver accept()
        {
            return new TransceiverI(_instance, _delegate.accept(), _adapterName, true);
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

        internal AcceptorI(EndpointI endpoint, Instance instance, IceInternal.Acceptor del, string adapterName)
        {
            _endpoint = endpoint;
            _delegate = del;
            _instance = instance;
            _adapterName = adapterName;

            //
            // .NET requires that a certificate be supplied.
            //
            var certs = instance.certs();
            if(certs == null || certs.Count == 0)
            {
                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: certificate required for server endpoint";
                throw ex;
            }
        }

        private EndpointI _endpoint;
        private IceInternal.Acceptor _delegate;
        private Instance _instance;
        private string _adapterName;
    }
}
