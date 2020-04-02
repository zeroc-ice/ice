//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace IceSSL
{
    internal sealed class Endpoint : IceInternal.Endpoint
    {
        internal Endpoint(Instance instance, IceInternal.Endpoint del)
        {
            _instance = instance;
            _delegate = del;
        }

        public override void StreamWriteImpl(Ice.OutputStream os) => _delegate.StreamWriteImpl(os);

        private sealed class InfoI : EndpointInfo
        {
            public InfoI(Endpoint e) => _endpoint = e;
            public override Ice.EndpointType Type() => _endpoint.Type();
            public override bool Datagram() => _endpoint.Datagram();
            public override bool Secure() => _endpoint.Secure();

            private readonly Endpoint _endpoint;
        }

        public override Ice.EndpointInfo GetInfo()
        {
            var info = new InfoI(this);
            info.Underlying = _delegate.GetInfo();
            info.Compress = info.Underlying.Compress;
            info.Timeout = info.Underlying.Timeout;
            return info;
        }

        public override Ice.EndpointType Type() => _delegate.Type();

        public override string Transport() => _delegate.Transport();

        public override int Timeout() => _delegate.Timeout();

        public override IceInternal.Endpoint Timeout(int timeout)
        {
            if (timeout == _delegate.Timeout())
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, _delegate.Timeout(timeout));
            }
        }

        public override string ConnectionId() => _delegate.ConnectionId();

        public override IceInternal.Endpoint ConnectionId(string connectionId)
        {
            if (connectionId.Equals(_delegate.ConnectionId()))
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, _delegate.ConnectionId(connectionId));
            }
        }

        public override bool Compress() => _delegate.Compress();

        public override IceInternal.Endpoint Compress(bool compress)
        {
            if (compress == _delegate.Compress())
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, _delegate.Compress(compress));
            }
        }

        public override bool Datagram() => _delegate.Datagram();
        public override bool Secure() => _delegate.Secure();
        public override IceInternal.ITransceiver? Transceiver() => null;

        private sealed class EndpointI_connectorsI : IceInternal.IEndpointConnectors
        {
            public EndpointI_connectorsI(Instance instance, string host, IceInternal.IEndpointConnectors cb)
            {
                _instance = instance;
                _host = host;
                _callback = cb;
            }

            public void Connectors(List<IceInternal.IConnector> connectors)
            {
                var l = new List<IceInternal.IConnector>();
                foreach (IceInternal.IConnector c in connectors)
                {
                    l.Add(new ConnectorI(_instance, c, _host));
                }
                _callback.Connectors(l);
            }

            public void Exception(System.Exception ex) => _callback.Exception(ex);

            private readonly Instance _instance;
            private readonly string _host;
            private readonly IceInternal.IEndpointConnectors _callback;
        }

        public override void ConnectorsAsync(Ice.EndpointSelectionType selType,
                                             IceInternal.IEndpointConnectors callback)
        {
            string host = "";
            for (Ice.EndpointInfo? p = _delegate.GetInfo(); p != null; p = p.Underlying)
            {
                if (p is Ice.IPEndpointInfo)
                {
                    host = ((Ice.IPEndpointInfo)p).Host;
                    break;
                }
            }
            _delegate.ConnectorsAsync(selType, new EndpointI_connectorsI(_instance, host, callback));
        }

        public override IceInternal.IAcceptor Acceptor(string adapterName) =>
            new Acceptor(this, _instance, _delegate.Acceptor(adapterName), adapterName);

        public Endpoint endpoint(IceInternal.Endpoint del)
        {
            if (del == _delegate)
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, del);
            }
        }

        public override List<IceInternal.Endpoint> ExpandIfWildcard()
        {
            var l = new List<IceInternal.Endpoint>();
            foreach (IceInternal.Endpoint e in _delegate.ExpandIfWildcard())
            {
                l.Add(e == _delegate ? this : new Endpoint(_instance, e));
            }
            return l;
        }

        public override List<IceInternal.Endpoint> ExpandHost(out IceInternal.Endpoint? publish)
        {
            var l = new List<IceInternal.Endpoint>();
            foreach (IceInternal.Endpoint e in _delegate.ExpandHost(out publish))
            {
                l.Add(e == _delegate ? this : new Endpoint(_instance, e));
            }
            if (publish != null)
            {
                publish = publish == _delegate ? this : new Endpoint(_instance, publish);
            }
            return l;
        }

        public override bool Equivalent(IceInternal.Endpoint endpoint)
        {
            if (!(endpoint is Endpoint))
            {
                return false;
            }
            var endpointI = (Endpoint)endpoint;
            return _delegate.Equivalent(endpointI._delegate);
        }

        public override string Options() => _delegate.Options();

        //
        // Compare endpoints for sorting purposes
        //
        public override int CompareTo(IceInternal.Endpoint obj)
        {
            if (!(obj is Endpoint))
            {
                return Type() < obj.Type() ? -1 : 1;
            }

            var p = (Endpoint)obj;
            if (this == p)
            {
                return 0;
            }

            return _delegate.CompareTo(p._delegate);
        }

        public override int GetHashCode() => _delegate.GetHashCode();

        protected override bool CheckOption(string option, string? argument, string endpoint) => false;

        private readonly Instance _instance;
        private readonly IceInternal.Endpoint _delegate;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactoryWithUnderlying
    {
        public EndpointFactoryI(Instance instance, Ice.EndpointType type) : base(instance, type) => _instance = instance;

        public override IceInternal.IEndpointFactory
        CloneWithUnderlying(IceInternal.TransportInstance inst, Ice.EndpointType underlying) =>
            new EndpointFactoryI(new Instance(_instance.Engine(), inst.Type, inst.Transport), underlying);

        protected override IceInternal.Endpoint
        CreateWithUnderlying(IceInternal.Endpoint? underlying, List<string> args, bool oaEndpoint) =>
            new Endpoint(_instance, underlying);

        protected override IceInternal.Endpoint
        ReadWithUnderlying(IceInternal.Endpoint? underlying, Ice.InputStream s) => new Endpoint(_instance, underlying);

        private readonly Instance _instance;
    }
}
