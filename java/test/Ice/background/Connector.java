// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

final class Connector implements IceInternal.Connector, java.lang.Comparable
{
    public IceInternal.Transceiver
    connect()
    {
        _configuration.checkConnectException();
        return new Transceiver(_connector.connect());
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
    Connector(IceInternal.Connector connector)
    {
        _configuration = Configuration.getInstance();
        _connector = connector;
    }

    //
    // Compare connectors for sorting purposes
    //
    public boolean
    equals(java.lang.Object obj)
    {
        return compareTo(obj) == 0;
    }

    public int
    compareTo(java.lang.Object obj) // From java.lang.Comparable
    {
        Connector p = null;

        try
        {
            p = (Connector)obj;
        }
        catch(ClassCastException ex)
        {
            try
            {
                IceInternal.Connector c = (IceInternal.Connector)obj;
                return type() < c.type() ? -1 : 1;
            }
            catch(ClassCastException ee)
            {
                assert(false);
            }
        }

        if(this == p)
        {
            return 0;
        }

        return _connector.compareTo(p._connector);
    } 

    protected synchronized void
    finalize()
        throws Throwable
    {
        super.finalize();
    }

    final private IceInternal.Connector _connector;
    final private Configuration _configuration;
}
