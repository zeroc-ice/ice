// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.background;

final class Connector implements IceInternal.Connector
{
    public IceInternal.Transceiver
    connect()
    {
        _configuration.checkConnectException();
        return new Transceiver(_configuration, _connector.connect());
    }

    public short
    type()
    {
        return (short)(EndpointI.TYPE_BASE + _connector.type());
    }

    public String
    toString()
    {
        return _connector.toString();
    }

    public int
    hashCode()
    {
        return _connector.hashCode();
    }

    //
    // Only for use by Endpoint
    //
    Connector(Configuration configuration, IceInternal.Connector connector)
    {
        _configuration = configuration;
        _connector = connector;
    }

    public boolean
    equals(java.lang.Object obj)
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

    final private IceInternal.Connector _connector;
    final private Configuration _configuration;
}
