// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.background;

final class Connector implements com.zeroc.IceInternal.Connector
{
    @Override
    public com.zeroc.IceInternal.Transceiver connect()
    {
        _configuration.checkConnectException();
        return new Transceiver(_configuration, _connector.connect());
    }

    @Override
    public short type()
    {
        return (short)(EndpointI.TYPE_BASE + _connector.type());
    }

    @Override
    public String toString()
    {
        return _connector.toString();
    }

    @Override
    public int hashCode()
    {
        return _connector.hashCode();
    }

    //
    // Only for use by Endpoint
    //
    Connector(Configuration configuration, com.zeroc.IceInternal.Connector connector)
    {
        _configuration = configuration;
        _connector = connector;
    }

    @Override
    public boolean equals(java.lang.Object obj)
    {
        Connector p = null;

        try
        {
            p = (Connector)obj;
        }
        catch(ClassCastException ex)
        {
            return false;
        }

        if(this == p)
        {
            return true;
        }

        return _connector.equals(p._connector);
    }

    final private com.zeroc.IceInternal.Connector _connector;
    final private Configuration _configuration;
}
