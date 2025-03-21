// Copyright (c) ZeroC, Inc.

internal class Connector : Ice.Internal.Connector
{
    public Ice.Internal.Transceiver connect()
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
    internal Connector(Ice.Internal.Connector connector)
    {
        _configuration = Configuration.getInstance();
        _connector = connector;
    }

    public override bool Equals(object obj)
    {
        Connector p = null;

        try
        {
            p = (Connector)obj;
        }
        catch (System.InvalidCastException)
        {
            return false;
        }

        if (this == p)
        {
            return true;
        }

        return _connector.Equals(p._connector);
    }

    public override string ToString()
    {
        return _connector.ToString();
    }

    public override int GetHashCode()
    {
        return _connector.GetHashCode();
    }

    private readonly Ice.Internal.Connector _connector;
    private readonly Configuration _configuration;
}
