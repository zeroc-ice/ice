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

        public override void streamWriteImpl(Ice.OutputStream os) => _delegate.streamWriteImpl(os);

        private sealed class InfoI : EndpointInfo
        {
            public InfoI(Endpoint e) => _endpoint = e;
            public override short type() =>  _endpoint.type();
            public override bool datagram() =>  _endpoint.datagram();
            public override bool secure() =>   _endpoint.secure();

            private readonly Endpoint _endpoint;
        }

        public override Ice.EndpointInfo getInfo()
        {
            InfoI info = new InfoI(this);
            info.underlying = _delegate.getInfo();
            info.compress = info.underlying.compress;
            info.timeout = info.underlying.timeout;
            return info;
        }

        public override short type() =>  _delegate.type();

        public override string protocol() => _delegate.protocol();

        public override int timeout() => _delegate.timeout();

        public override IceInternal.Endpoint timeout(int timeout)
        {
            if (timeout == _delegate.timeout())
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, _delegate.timeout(timeout));
            }
        }

        public override string connectionId() => _delegate.connectionId();

        public override IceInternal.Endpoint connectionId(string connectionId)
        {
            if (connectionId.Equals(_delegate.connectionId()))
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, _delegate.connectionId(connectionId));
            }
        }

        public override bool compress() => _delegate.compress();

        public override IceInternal.Endpoint compress(bool compress)
        {
            if (compress == _delegate.compress())
            {
                return this;
            }
            else
            {
                return new Endpoint(_instance, _delegate.compress(compress));
            }
        }

        public override bool datagram() =>  _delegate.datagram();
        public override bool secure() =>  _delegate.secure();
        public override IceInternal.ITransceiver? transceiver() =>  null;

        private sealed class EndpointI_connectorsI : IceInternal.IEndpointConnectors
        {
            public EndpointI_connectorsI(Instance instance, string host, IceInternal.IEndpointConnectors cb)
            {
                _instance = instance;
                _host = host;
                _callback = cb;
            }

            public void connectors(List<IceInternal.IConnector> connectors)
            {
                List<IceInternal.IConnector> l = new List<IceInternal.IConnector>();
                foreach (IceInternal.IConnector c in connectors)
                {
                    l.Add(new ConnectorI(_instance, c, _host));
                }
                _callback.connectors(l);
            }

            public void exception(Ice.LocalException ex) => _callback.exception(ex);

            private readonly Instance _instance;
            private readonly string _host;
            private readonly IceInternal.IEndpointConnectors _callback;
        }

        public override void ConnectorsAsync(Ice.EndpointSelectionType selType,
                                             IceInternal.IEndpointConnectors callback)
        {
            string host = "";
            for (Ice.EndpointInfo? p = _delegate.getInfo(); p != null; p = p.underlying)
            {
                if (p is Ice.IPEndpointInfo)
                {
                    host = ((Ice.IPEndpointInfo)p).host;
                    break;
                }
            }
            _delegate.ConnectorsAsync(selType, new EndpointI_connectorsI(_instance, host, callback));
        }

        public override IceInternal.IAcceptor acceptor(string adapterName) =>
            new Acceptor(this, _instance, _delegate.acceptor(adapterName), adapterName);

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

        public override List<IceInternal.Endpoint> expandIfWildcard()
        {
            List<IceInternal.Endpoint> l = new List<IceInternal.Endpoint>();
            foreach (IceInternal.Endpoint e in _delegate.expandIfWildcard())
            {
                l.Add(e == _delegate ? this : new Endpoint(_instance, e));
            }
            return l;
        }

        public override List<IceInternal.Endpoint> expandHost(out IceInternal.Endpoint? publish)
        {
            List<IceInternal.Endpoint> l = new List<IceInternal.Endpoint>();
            foreach (IceInternal.Endpoint e in _delegate.expandHost(out publish))
            {
                l.Add(e == _delegate ? this : new Endpoint(_instance, e));
            }
            if (publish != null)
            {
                publish = publish == _delegate ? this : new Endpoint(_instance, publish);
            }
            return l;
        }

        public override bool equivalent(IceInternal.Endpoint endpoint)
        {
            if (!(endpoint is Endpoint))
            {
                return false;
            }
            Endpoint endpointI = (Endpoint)endpoint;
            return _delegate.equivalent(endpointI._delegate);
        }

        public override string options() =>  _delegate.options();

        //
        // Compare endpoints for sorting purposes
        //
        public override int CompareTo(IceInternal.Endpoint obj)
        {
            if (!(obj is Endpoint))
            {
                return type() < obj.type() ? -1 : 1;
            }

            Endpoint p = (Endpoint)obj;
            if (this == p)
            {
                return 0;
            }

            return _delegate.CompareTo(p._delegate);
        }

        public override int GetHashCode() =>  _delegate.GetHashCode();

        protected override bool checkOption(string option, string? argument, string endpoint) => false;

        private readonly Instance _instance;
        private readonly IceInternal.Endpoint _delegate;
    }

    internal sealed class EndpointFactoryI : IceInternal.EndpointFactoryWithUnderlying
    {
        public EndpointFactoryI(Instance instance, short type) : base(instance, type) => _instance = instance;

        public override IceInternal.IEndpointFactory
        cloneWithUnderlying(IceInternal.ProtocolInstance inst, short underlying) =>
            new EndpointFactoryI(new Instance(_instance.engine(), inst.Type, inst.Protocol), underlying);

        protected override IceInternal.Endpoint
        createWithUnderlying(IceInternal.Endpoint underlying, List<string> args, bool oaEndpoint) =>
            new Endpoint(_instance, underlying);

        protected override IceInternal.Endpoint
        readWithUnderlying(IceInternal.Endpoint underlying, Ice.InputStream s) =>  new Endpoint(_instance, underlying);

        private readonly Instance _instance;
    }
}
