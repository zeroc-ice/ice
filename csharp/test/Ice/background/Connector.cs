//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

internal class Connector : IceInternal.Connector
{
    public IceInternal.Transceiver connect()
    {
        _configuration.checkConnectException();
        return new Transceiver(_connector.connect());
    }

    public short type()
    {
        return (short)(EndpointI.TYPE_BASE + _connector.type());
    }

    //
    // Only for use by Endpoint
    //
    internal Connector(IceInternal.Connector connector)
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

    private IceInternal.Connector _connector;
    private Configuration _configuration;
}
