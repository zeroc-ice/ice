//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal class SslAcceptor : IAcceptor
    {
        public void Close() => _delegate.Close();

        public Endpoint Listen()
        {
            _endpoint = _endpoint.GetEndpoint(_delegate.Listen());
            return _endpoint;
        }

        public bool StartAccept(AsyncCallback callback, object state)
        {
            //
            // The plug-in may not be fully initialized.
            //
            if (!_instance.Initialized())
            {
                throw new System.InvalidOperationException("IceSSL: plug-in is not initialized");
            }
            return _delegate.StartAccept(callback, state);
        }

        public void FinishAccept() => _delegate.FinishAccept();

        public ITransceiver Accept() => new SslTransceiver(_instance, _delegate.Accept(), _adapterName, true);

        public string Transport() => _delegate.Transport();

        public override string ToString() => _delegate.ToString();

        public string ToDetailedString() => _delegate.ToDetailedString();

        internal SslAcceptor(SslEndpoint endpoint, SslInstance instance, IAcceptor del, string adapterName)
        {
            _endpoint = endpoint;
            _delegate = del;
            _instance = instance;
            _adapterName = adapterName;

            //
            // .NET requires that a certificate be supplied.
            //
            System.Security.Cryptography.X509Certificates.X509Certificate2Collection? certs = instance.Certs();
            if (certs == null || certs.Count == 0)
            {
                throw new SecurityException("IceSSL: certificate required for server endpoint");
            }
        }

        private SslEndpoint _endpoint;
        private readonly IAcceptor _delegate;
        private readonly SslInstance _instance;
        private readonly string _adapterName;
    }
}
