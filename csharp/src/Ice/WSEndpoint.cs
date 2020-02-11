//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.Diagnostics;

namespace IceInternal
{
    internal sealed class WSEndpoint : Endpoint
    {
        internal WSEndpoint(ProtocolInstance instance, Endpoint del, string res)
        {
            _instance = instance;
            _delegate = del;
            _resource = res;
        }

#pragma warning disable CS8618 // Non-nullable field is uninitialized. Consider declaring as nullable.
        internal WSEndpoint(ProtocolInstance instance, Endpoint del, List<string> args)
#pragma warning restore CS8618 // Non-nullable field is uninitialized. Consider declaring as nullable.
        {
            _instance = instance;
            _delegate = del;

            InitWithOptions(args);

            if (_resource == null)
            {
                _resource = "/";
            }
        }

        internal WSEndpoint(ProtocolInstance instance, Endpoint del, Ice.InputStream s)
        {
            _instance = instance;
            _delegate = del;

            _resource = s.ReadString();
        }

        private sealed class InfoI : Ice.WSEndpointInfo
        {
            public InfoI(Endpoint e) => _endpoint = e;

            public override short Type() => _endpoint.Type();

            public override bool Datagram() => _endpoint.Datagram();

            public override bool Secure() => _endpoint.Secure();

            private readonly Endpoint _endpoint;
        }

        public override Ice.EndpointInfo GetInfo()
        {
            Ice.WSEndpointInfo info = new InfoI(this);
            info.Underlying = _delegate.GetInfo();
            info.Resource = _resource;
            info.Compress = info.Underlying.Compress;
            info.Timeout = info.Underlying.Timeout;
            return info;
        }

        public override short Type() => _delegate.Type();

        public override string Protocol() => _delegate.Protocol();

        public override void StreamWriteImpl(Ice.OutputStream s)
        {
            _delegate.StreamWriteImpl(s);
            s.WriteString(_resource);
        }

        public override int Timeout() => _delegate.Timeout();

        public override Endpoint Timeout(int timeout)
        {
            if (timeout == _delegate.Timeout())
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.Timeout(timeout), _resource);
            }
        }

        public override string ConnectionId() => _delegate.ConnectionId();

        public override Endpoint ConnectionId(string connectionId)
        {
            if (connectionId.Equals(_delegate.ConnectionId()))
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.ConnectionId(connectionId), _resource);
            }
        }

        public override bool Compress() => _delegate.Compress();

        public override Endpoint Compress(bool compress)
        {
            if (compress == _delegate.Compress())
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.Compress(compress), _resource);
            }
        }

        public override bool Datagram() => _delegate.Datagram();

        public override bool Secure() => _delegate.Secure();

        public override ITransceiver? Transceiver() => null;

        private sealed class EndpointConnectors : IEndpointConnectors
        {
            public EndpointConnectors(ProtocolInstance instance, string host, string res, IEndpointConnectors cb)
            {
                _instance = instance;
                _host = host;
                _resource = res;
                _callback = cb;
            }

            public void Connectors(List<IConnector> connectors)
            {
                var l = new List<IConnector>();
                foreach (IConnector c in connectors)
                {
                    l.Add(new WSConnector(_instance, c, _host, _resource));
                }
                _callback.Connectors(l);
            }

            public void Exception(Ice.LocalException ex) => _callback.Exception(ex);

            private readonly ProtocolInstance _instance;
            private readonly string _host;
            private readonly string _resource;
            private readonly IEndpointConnectors _callback;
        }

        public override void ConnectorsAsync(Ice.EndpointSelectionType selType, IEndpointConnectors callback)
        {
            string host = "";
            for (Ice.EndpointInfo? p = _delegate.GetInfo(); p != null; p = p.Underlying)
            {
                if (p is Ice.IPEndpointInfo ipInfo)
                {
                    host = ipInfo.Host + ":" + ipInfo.Port;
                    break;
                }
            }
            _delegate.ConnectorsAsync(selType, new EndpointConnectors(_instance, host, _resource, callback));
        }

        public override IAcceptor Acceptor(string adapterName)
        {
            IAcceptor? acceptor = _delegate.Acceptor(adapterName);
            Debug.Assert(acceptor != null);
            return new WSAcceptor(this, _instance, acceptor);
        }

        public WSEndpoint endpoint(Endpoint delEndp)
        {
            if (delEndp == _delegate)
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, delEndp, _resource);
            }
        }

        public override List<Endpoint> ExpandIfWildcard()
        {
            var l = new List<Endpoint>();
            foreach (Endpoint e in _delegate.ExpandIfWildcard())
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            return l;
        }

        public override List<Endpoint> ExpandHost(out Endpoint? publish)
        {
            var l = new List<Endpoint>();
            foreach (Endpoint e in _delegate.ExpandHost(out publish))
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            if (publish != null)
            {
                publish = publish == _delegate ? this : new WSEndpoint(_instance, publish, _resource);
            }
            return l;
        }

        public override bool Equivalent(Endpoint endpoint)
        {
            if (!(endpoint is WSEndpoint))
            {
                return false;
            }
            var wsEndpointI = (WSEndpoint)endpoint;
            return _delegate.Equivalent(wsEndpointI._delegate);
        }

        public override string Options()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = _delegate.Options();

            if (_resource != null && _resource.Length > 0)
            {
                s += " -r ";
                bool addQuote = _resource.IndexOf(':') != -1;
                if (addQuote)
                {
                    s += "\"";
                }
                s += _resource;
                if (addQuote)
                {
                    s += "\"";
                }
            }

            return s;
        }

        public override int GetHashCode() => HashCode.Combine(_delegate, _resource);

        public override int CompareTo(Endpoint obj)
        {
            if (!(obj is WSEndpoint))
            {
                return Type() < obj.Type() ? -1 : 1;
            }

            var p = (WSEndpoint)obj;
            if (this == p)
            {
                return 0;
            }

            int v = string.Compare(_resource, p._resource, StringComparison.Ordinal);
            if (v != 0)
            {
                return v;
            }

            return _delegate.CompareTo(p._delegate);
        }

        protected override bool CheckOption(string option, string? argument, string endpoint)
        {
            switch (option[1])
            {
                case 'r':
                    {
                        _resource = argument ?? throw new FormatException(
                                $"no argument provided for -r option in endpoint {endpoint}{_delegate.Options()}");
                        return true;
                    }
                default:
                    {
                        return false;
                    }
            }
        }

        private readonly ProtocolInstance _instance;
        private readonly Endpoint _delegate;
        private string _resource;
    }

    public class WSEndpointFactory : EndpointFactoryWithUnderlying
    {
        public WSEndpointFactory(ProtocolInstance instance, short type) : base(instance, type)
        {
        }

        public override IEndpointFactory CloneWithUnderlying(ProtocolInstance instance, short underlying) =>
            new WSEndpointFactory(instance, underlying);

        protected override Endpoint CreateWithUnderlying(Endpoint underlying, List<string> args, bool oaEndpoint) =>
            new WSEndpoint(Instance!, underlying, args);

        protected override Endpoint ReadWithUnderlying(Endpoint underlying, Ice.InputStream s) =>
            new WSEndpoint(Instance!, underlying, s);
    }
}
