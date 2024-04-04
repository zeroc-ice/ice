// Copyright (c) ZeroC, Inc.

using System.Net.Security;

namespace IceSSL;

internal sealed class EndpointI : IceInternal.EndpointI
{
    internal EndpointI(Instance instance, IceInternal.EndpointI del)
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

    public override IceInternal.EndpointI timeout(int timeout)
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

    public override IceInternal.EndpointI connectionId(string connectionId)
    {
        if (connectionId.Equals(_delegate.connectionId()))
        {
            return this;
        }
        else
        {
            return new EndpointI(_instance, _delegate.connectionId(connectionId));
        }
    }

    public override bool compress() => _delegate.compress();

    public override IceInternal.EndpointI compress(bool compress)
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

    public override IceInternal.Transceiver transceiver() => null;

    private sealed class EndpointI_connectorsI : IceInternal.EndpointI_connectors
    {
        public EndpointI_connectorsI(Instance instance, string host, IceInternal.EndpointI_connectors cb)
        {
            _instance = instance;
            _host = host;
            _callback = cb;
        }

        public void connectors(List<IceInternal.Connector> connectors)
        {
            var l = new List<IceInternal.Connector>();
            foreach (IceInternal.Connector c in connectors)
            {
                l.Add(new ConnectorI(_instance, c, _host));
            }
            _callback.connectors(l);
        }

        public void exception(Ice.LocalException ex) => _callback.exception(ex);

        private readonly Instance _instance;
        private readonly string _host;
        private readonly IceInternal.EndpointI_connectors _callback;
    }

    public override void connectors_async(Ice.EndpointSelectionType selType,
                                          IceInternal.EndpointI_connectors callback)
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

    public override IceInternal.Acceptor acceptor(
        string adapterName,
        SslServerAuthenticationOptions authenticationOptions) =>
        new AcceptorI(
            this,
            _instance,
            _delegate.acceptor(adapterName, null),
            adapterName,
            authenticationOptions);

    public EndpointI endpoint(IceInternal.EndpointI del)
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

    public override List<IceInternal.EndpointI> expandIfWildcard()
    {
        var l = new List<IceInternal.EndpointI>();
        foreach (IceInternal.EndpointI e in _delegate.expandIfWildcard())
        {
            l.Add(e == _delegate ? this : new EndpointI(_instance, e));
        }
        return l;
    }

    public override List<IceInternal.EndpointI> expandHost(out IceInternal.EndpointI publish)
    {
        var l = new List<IceInternal.EndpointI>();
        foreach (IceInternal.EndpointI e in _delegate.expandHost(out publish))
        {
            l.Add(e == _delegate ? this : new EndpointI(_instance, e));
        }
        if (publish != null)
        {
            publish = publish == _delegate ? this : new EndpointI(_instance, publish);
        }
        return l;
    }

    public override bool equivalent(IceInternal.EndpointI endpoint)
    {
        if (endpoint is not EndpointI)
        {
            return false;
        }
        var endpointI = (EndpointI)endpoint;
        return _delegate.equivalent(endpointI._delegate);
    }

    public override string options() => _delegate.options();

    //
    // Compare endpoints for sorting purposes
    //
    public override int CompareTo(IceInternal.EndpointI obj)
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
    private readonly IceInternal.EndpointI _delegate;
}

internal sealed class EndpointFactoryI : IceInternal.EndpointFactoryWithUnderlying
{
    public EndpointFactoryI(Instance instance, short type) : base(instance, type) => _instance = instance;

    public override IceInternal.EndpointFactory
    cloneWithUnderlying(IceInternal.ProtocolInstance instance, short underlying) =>
        new EndpointFactoryI(new Instance(_instance.engine(), instance.type(), instance.protocol()), underlying);

    protected override IceInternal.EndpointI
    createWithUnderlying(IceInternal.EndpointI underlying, List<string> args, bool oaEndpoint) =>
        new EndpointI(_instance, underlying);

    protected override IceInternal.EndpointI
    readWithUnderlying(IceInternal.EndpointI underlying, Ice.InputStream s) => new EndpointI(_instance, underlying);

    private readonly Instance _instance;
}
