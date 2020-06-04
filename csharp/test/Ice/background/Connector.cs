//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

namespace ZeroC.Ice.Test.Background
{
    internal class Connector : IConnector
    {
        public ITransceiver Connect()
        {
            _configuration.CheckConnectException();
            return new Transceiver(_connector.Connect());
        }

        public string Transport => "";
        public EndpointType Type => (EndpointType)(Endpoint.TYPE_BASE + (short)_connector.Type);

        //
        // Only for use by Endpoint
        //
        internal Connector(IConnector connector)
        {
            _configuration = Configuration.GetInstance();
            _connector = connector;
        }

        public override bool Equals(object? obj)
        {
            if (ReferenceEquals(this, obj))
            {
                return true;
            }

            if (!(obj is Connector))
            {
                return false;
            }
            return _connector.Equals(((Connector)obj)._connector);
        }

        public override string? ToString() => _connector.ToString();

        public override int GetHashCode() => _connector.GetHashCode();

        private readonly IConnector _connector;
        private readonly Configuration _configuration;
    }
}
