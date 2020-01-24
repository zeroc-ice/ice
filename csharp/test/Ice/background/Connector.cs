//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Connector : IceInternal.IConnector
{
    public IceInternal.ITransceiver Connect()
    {
        _configuration.checkConnectException();
        return new Transceiver(_connector.Connect());
    }

    public short Type()
    {
        return (short)(Endpoint.TYPE_BASE + _connector.Type());
    }

    //
    // Only for use by Endpoint
    //
    internal Connector(IceInternal.IConnector connector)
    {
        _configuration = Configuration.getInstance();
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

    public override string? ToString()
    {
        return _connector.ToString();
    }

    public override int GetHashCode()
    {
        return _connector.GetHashCode();
    }

    private IceInternal.IConnector _connector;
    private Configuration _configuration;
}
