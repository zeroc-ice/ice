//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice
{
    internal sealed class SslConnector : IConnector
    {
        public ITransceiver Connect()
        {
            //
            // The plug-in may not be fully initialized.
            //
            if (!_instance.Initialized())
            {
                throw new System.InvalidOperationException("IceSSL: plug-in is not initialized");
            }

            return new SslTransceiver(_instance, _delegate.Connect(), _host, false);
        }

        public EndpointType Type() => _delegate.Type();

        //
        // Only for use by EndpointI.
        //
        internal SslConnector(SslInstance instance, IConnector del, string host)
        {
            _instance = instance;
            _delegate = del;
            _host = host;
        }

        public override bool Equals(object? obj)
        {
            if (!(obj is SslConnector))
            {
                return false;
            }

            if (this == obj)
            {
                return true;
            }

            var p = (SslConnector)obj;
            return _delegate.Equals(p._delegate);
        }

        public override string ToString() => _delegate.ToString()!;

        public override int GetHashCode() => _delegate.GetHashCode();

        private readonly SslInstance _instance;
        private readonly IConnector _delegate;
        private readonly string _host;
    }
}
