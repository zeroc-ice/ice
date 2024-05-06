// Copyright (c) ZeroC, Inc.

namespace Ice.SSL;

internal sealed class ConnectorI : Ice.Internal.Connector
{
    public Ice.Internal.Transceiver connect() =>
        new TransceiverI(
            _instance,
            _delegate.connect(),
            _host,
            incoming: false,
            serverAuthenticationOptions: null);

    public short type() => _delegate.type();

    public override bool Equals(object obj)
    {
        if (obj is not ConnectorI)
        {
            return false;
        }

        if (this == obj)
        {
            return true;
        }

        var p = (ConnectorI)obj;
        return _delegate.Equals(p._delegate);
    }

    public override int GetHashCode() => _delegate.GetHashCode();

    public override string ToString() => _delegate.ToString();

    // Only for use by EndpointI.
    internal ConnectorI(Instance instance, Ice.Internal.Connector del, string host)
    {
        _instance = instance;
        _delegate = del;
        _host = host;
    }

    private readonly Ice.Internal.Connector _delegate;
    private readonly string _host;
    private readonly Instance _instance;
}
