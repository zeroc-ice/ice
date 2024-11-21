// Copyright (c) ZeroC, Inc.

using System.Net.Security;

namespace Ice.SSL;

internal sealed class EndpointI : Ice.Internal.EndpointI
{
    internal EndpointI(Instance instance, Ice.Internal.EndpointI del)
    {
        _instance = instance;
        _delegate = del;
    }

    public override void streamWriteImpl(Ice.OutputStream os) => _delegate.streamWriteImpl(os);

    private sealed class InfoI : EndpointInfo
    {
        public InfoI(EndpointI e) => _endpoint = e;

        public override short type() => _endpoint.type();

        public override bool datagram() => _endpoint.datagram();

        public override bool secure() => _endpoint.secure();

        private readonly EndpointI _endpoint;
    }

    public override Ice.EndpointInfo getInfo()
    {
        var info = new InfoI(this);
        info.underlying = _delegate.getInfo();
        info.compress = info.underlying.compress;
        info.timeout = info.underlying.timeout;
        return info;
    }

    public override short type() => _delegate.type();

    public override string protocol() => _delegate.protocol();

    public override int timeout() => _delegate.timeout();

    public override Ice.Internal.EndpointI timeout(int timeout)
    {
        if (timeout == _delegate.timeout())
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _delegate.timeout(timeout));
        }
    }

    public override string connectionId() => _delegate.connectionId();

    public override Ice.Internal.EndpointI connectionId(string connectionId)
    {
        if (connectionId.Equals(_delegate.connectionId(), StringComparison.Ordinal))
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _delegate.connectionId(connectionId));
        }
    }

    public override bool compress() => _delegate.compress();

    public override Ice.Internal.EndpointI compress(bool compress)
    {
        if (compress == _delegate.compress())
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _delegate.compress(compress));
        }
    }

    public override bool datagram() => _delegate.datagram();

    public override bool secure() => _delegate.secure();

    public override Ice.Internal.Transceiver transceiver() => null;

    private sealed class EndpointI_connectorsI : Ice.Internal.EndpointI_connectors
    {
        public EndpointI_connectorsI(Instance instance, string host, Ice.Internal.EndpointI_connectors cb)
        {
            _instance = instance;
            _host = host;
            _callback = cb;
        }

        public void connectors(List<Ice.Internal.Connector> connectors)
        {
            var l = new List<Ice.Internal.Connector>();
            foreach (Ice.Internal.Connector c in connectors)
            {
                l.Add(new ConnectorI(_instance, c, _host));
            }
            _callback.connectors(l);
        }

        public void exception(Ice.LocalException ex) => _callback.exception(ex);

        private readonly Instance _instance;
        private readonly string _host;
        private readonly Ice.Internal.EndpointI_connectors _callback;
    }

    public override void connectors_async(
        Ice.EndpointSelectionType selType,
        Ice.Internal.EndpointI_connectors callback)
    {
        string host = "";
        for (Ice.EndpointInfo p = _delegate.getInfo(); p != null; p = p.underlying)
        {
            if (p is Ice.IPEndpointInfo info)
            {
                host = info.host;
                break;
            }
        }
        _delegate.connectors_async(selType, new EndpointI_connectorsI(_instance, host, callback));
    }

    public override Ice.Internal.Acceptor acceptor(
        string adapterName,
        SslServerAuthenticationOptions authenticationOptions) =>
        new AcceptorI(
            this,
            _instance,
            _delegate.acceptor(adapterName, null),
            adapterName,
            authenticationOptions);

    public EndpointI endpoint(Ice.Internal.EndpointI del)
    {
        if (del == _delegate)
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, del);
        }
    }

    public override List<Internal.EndpointI> expandHost() =>
        _delegate.expandHost().Select(e => endpoint(e) as Internal.EndpointI).ToList();

    public override bool isLoopbackOrMulticast() => _delegate.isLoopbackOrMulticast();

    public override EndpointI withPublishedHost(string host) => endpoint(_delegate.withPublishedHost(host));

    public override bool equivalent(Ice.Internal.EndpointI endpoint)
    {
        if (endpoint is not EndpointI)
        {
            return false;
        }
        var endpointI = (EndpointI)endpoint;
        return _delegate.equivalent(endpointI._delegate);
    }

    public override string options() => _delegate.options();

    // Compare endpoints for sorting purposes
    public override int CompareTo(Ice.Internal.EndpointI obj)
    {
        if (obj is not EndpointI)
        {
            return type() < obj.type() ? -1 : 1;
        }

        var p = (EndpointI)obj;
        if (this == p)
        {
            return 0;
        }

        return _delegate.CompareTo(p._delegate);
    }

    public override int GetHashCode() => _delegate.GetHashCode();

    protected override bool checkOption(string option, string argument, string endpoint) => false;

    private readonly Instance _instance;
    private readonly Ice.Internal.EndpointI _delegate;
}

internal sealed class EndpointFactoryI : Ice.Internal.EndpointFactoryWithUnderlying
{
    public EndpointFactoryI(Instance instance, short type)
        : base(instance, type) => _instance = instance;

    public override Ice.Internal.EndpointFactory
    cloneWithUnderlying(Ice.Internal.ProtocolInstance instance, short underlying) =>
        new EndpointFactoryI(new Instance(_instance.engine(), instance.type(), instance.protocol()), underlying);

    protected override Ice.Internal.EndpointI
    createWithUnderlying(Ice.Internal.EndpointI underlying, List<string> args, bool oaEndpoint) =>
        new EndpointI(_instance, underlying);

    protected override Ice.Internal.EndpointI
    readWithUnderlying(Ice.Internal.EndpointI underlying, Ice.InputStream s) => new EndpointI(_instance, underlying);

    private readonly Instance _instance;
}
