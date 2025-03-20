// Copyright (c) ZeroC, Inc.

namespace Ice.Internal;

internal sealed class WSConnector : Connector
{
    public Transceiver connect() => new WSTransceiver(_instance, _delegate.connect(), _host, _resource);

    public short type() => _delegate.type();

    internal WSConnector(ProtocolInstance instance, Connector del, string host, string resource)
    {
        _instance = instance;
        _delegate = del;
        _host = host;
        _resource = resource;
    }

    public override bool Equals(object obj)
    {
        if (!(obj is WSConnector))
        {
            return false;
        }

        if (this == obj)
        {
            return true;
        }

        var p = (WSConnector)obj;
        if (!_delegate.Equals(p._delegate))
        {
            return false;
        }

        if (!_resource.Equals(p._resource, StringComparison.Ordinal))
        {
            return false;
        }

        return true;
    }

    public override string ToString() => _delegate.ToString();

    public override int GetHashCode() => _delegate.GetHashCode();

    private readonly ProtocolInstance _instance;
    private readonly Connector _delegate;
    private readonly string _host;
    private readonly string _resource;
}
