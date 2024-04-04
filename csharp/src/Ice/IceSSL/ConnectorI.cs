// Copyright (c) ZeroC, Inc.

namespace IceSSL;

internal sealed class ConnectorI : IceInternal.Connector
{
    public IceInternal.Transceiver connect() =>
        new TransceiverI(
            _instance,
            _delegate.connect(),
            _host,
            incoming: false,
            serverAuthenticationOptions: null);

    public short type() => _delegate.type();

    // Only for use by EndpointI.
    internal ConnectorI(Instance instance, IceInternal.Connector del, string host)
    {
        _instance = instance;
        _delegate = del;
        _host = host;
    }

    public override bool Equals(object obj)
    {
        if (!(obj is ConnectorI))
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

    public override string ToString() => _delegate.ToString();

    public override int GetHashCode() => _delegate.GetHashCode();

    private readonly IceInternal.Connector _delegate;
    private readonly string _host;
    private readonly Instance _instance;
}
