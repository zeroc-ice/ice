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

            initWithOptions(args);

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
            public InfoI(Endpoint e)
            {
                _endpoint = e;
            }

            public override short type()
            {
                return _endpoint.type();
            }

            public override bool datagram()
            {
                return _endpoint.datagram();
            }

            public override bool secure()
            {
                return _endpoint.secure();
            }

            private readonly Endpoint _endpoint;
        }

        public override Ice.EndpointInfo getInfo()
        {
            Ice.WSEndpointInfo info = new InfoI(this);
            info.underlying = _delegate.getInfo();
            info.resource = _resource;
            info.compress = info.underlying.compress;
            info.timeout = info.underlying.timeout;
            return info;
        }

        public override short type() => _delegate.type();

        public override string protocol() => _delegate.protocol();

        public override void streamWriteImpl(Ice.OutputStream s)
        {
            _delegate.streamWriteImpl(s);
            s.WriteString(_resource);
        }

        public override int timeout() => _delegate.timeout();

        public override Endpoint timeout(int timeout)
        {
            if (timeout == _delegate.timeout())
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.timeout(timeout), _resource);
            }
        }

        public override string connectionId() => _delegate.connectionId();

        public override Endpoint connectionId(string connectionId)
        {
            if (connectionId.Equals(_delegate.connectionId()))
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.connectionId(connectionId), _resource);
            }
        }

        public override bool compress() => _delegate.compress();

        public override Endpoint compress(bool compress)
        {
            if (compress == _delegate.compress())
            {
                return this;
            }
            else
            {
                return new WSEndpoint(_instance, _delegate.compress(compress), _resource);
            }
        }

        public override bool datagram() => _delegate.datagram();

        public override bool secure() => _delegate.secure();

        public override ITransceiver? transceiver() => null;

        private sealed class EndpointConnectors : IEndpointConnectors
        {
            public EndpointConnectors(ProtocolInstance instance, string host, string res, IEndpointConnectors cb)
            {
                _instance = instance;
                _host = host;
                _resource = res;
                _callback = cb;
            }

            public void connectors(List<IConnector> connectors)
            {
                List<IConnector> l = new List<IConnector>();
                foreach (IConnector c in connectors)
                {
                    l.Add(new WSConnector(_instance, c, _host, _resource));
                }
                _callback.connectors(l);
            }

            public void exception(Ice.LocalException ex)
            {
                _callback.exception(ex);
            }

            private readonly ProtocolInstance _instance;
            private readonly string _host;
            private readonly string _resource;
            private readonly IEndpointConnectors _callback;
        }

        public override void connectors_async(Ice.EndpointSelectionType selType, IEndpointConnectors callback)
        {
            string host = "";
            for (Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
            {
                if (p is Ice.IPEndpointInfo ipInfo)
                {
                    host = ipInfo.host + ":" + ipInfo.port;
                    break;
                }
            }
            _delegate.connectors_async(selType, new EndpointConnectors(_instance, host, _resource, callback));
        }

        public override IAcceptor acceptor(string adapterName)
        {
            var acceptor = _delegate.acceptor(adapterName);
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

        public override List<Endpoint> expandIfWildcard()
        {
            List<Endpoint> l = new List<Endpoint>();
            foreach (Endpoint e in _delegate.expandIfWildcard())
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            return l;
        }

        public override List<Endpoint> expandHost(out Endpoint? publish)
        {
            List<Endpoint> l = new List<Endpoint>();
            foreach (Endpoint e in _delegate.expandHost(out publish))
            {
                l.Add(e == _delegate ? this : new WSEndpoint(_instance, e, _resource));
            }
            if (publish != null)
            {
                publish = publish == _delegate ? this : new WSEndpoint(_instance, publish, _resource);
            }
            return l;
        }

        public override bool equivalent(Endpoint endpoint)
        {
            if (!(endpoint is WSEndpoint))
            {
                return false;
            }
            WSEndpoint wsEndpointI = (WSEndpoint)endpoint;
            return _delegate.equivalent(wsEndpointI._delegate);
        }

        public override string options()
        {
            //
            // WARNING: Certain features, such as proxy validation in Glacier2,
            // depend on the format of proxy strings. Changes to toString() and
            // methods called to generate parts of the reference string could break
            // these features. Please review for all features that depend on the
            // format of proxyToString() before changing this and related code.
            //
            string s = _delegate.options();

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

        public override int GetHashCode()
        {
            int h = _delegate.GetHashCode();
            HashUtil.hashAdd(ref h, _resource);
            return h;
        }

        public override int CompareTo(Endpoint obj)
        {
            if (!(obj is WSEndpoint))
            {
                return type() < obj.type() ? -1 : 1;
            }

            WSEndpoint p = (WSEndpoint)obj;
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

        protected override bool checkOption(string option, string? argument, string endpoint)
        {
            switch (option[1])
            {
                case 'r':
                    {
                        _resource = argument ?? throw new FormatException(
                                $"no argument provided for -r option in endpoint {endpoint}{_delegate.options()}");
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

        public override IEndpointFactory cloneWithUnderlying(ProtocolInstance instance, short underlying)
        {
            return new WSEndpointFactory(instance, underlying);
        }

        protected override Endpoint createWithUnderlying(Endpoint underlying, List<string> args, bool oaEndpoint)
        {
            return new WSEndpoint(instance_, underlying, args);
        }

        protected override Endpoint readWithUnderlying(Endpoint underlying, Ice.InputStream s)
        {
            return new WSEndpoint(instance_, underlying, s);
        }
    }
}
