//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace IceSSL
{
    using System.Net.Security;

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
            return _delegate.startAccept(callback, state);
        }

        public void finishAccept()
        {
            _delegate.finishAccept();
        }

        public IceInternal.Transceiver accept()
        {
            return new TransceiverI(
                _instance,
                _delegate.accept(),
                _adapterName,
                incoming: true,
                serverAuthenticationOptions: _serverAuthenticationOptions);
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

        internal AcceptorI(
            EndpointI endpoint,
            Instance instance,
            IceInternal.Acceptor del,
            string adapterName,
            SslServerAuthenticationOptions authenticationOptions)
        {
            _endpoint = endpoint;
            _delegate = del;
            _instance = instance;
            _adapterName = adapterName;
            _serverAuthenticationOptions = authenticationOptions;

            //
            // .NET requires that a certificate be supplied.
            //
            var certs = instance.certs();
            if((certs is null || certs.Count == 0) && _serverAuthenticationOptions is null)
            {
                throw new Ice.SecurityException("IceSSL: certificate required for server endpoint");
            }
        }

        private EndpointI _endpoint;
        private readonly IceInternal.Acceptor _delegate;
        private readonly Instance _instance;
        private readonly string _adapterName;
        private readonly SslServerAuthenticationOptions _serverAuthenticationOptions;
    }
}
