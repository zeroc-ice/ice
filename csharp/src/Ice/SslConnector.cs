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
            if (!_engine.Initialized())
            {
                throw new System.InvalidOperationException("IceSSL: plug-in is not initialized");
            }

            return new SslTransceiver(_communicator, _engine, _delegate.Connect(), _host, false);
        }

        public EndpointType Type => _delegate.Type;

        public string Transport => _delegate.Transport;

        //
        // Only for use by EndpointI.
        //
        internal SslConnector(Communicator communicator, SslEngine engine, IConnector del, string host)
        {
            _communicator = communicator;
            _engine = engine;
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

        private readonly Communicator _communicator;
        private readonly SslEngine _engine;
        private readonly IConnector _delegate;
        private readonly string _host;
    }
}
