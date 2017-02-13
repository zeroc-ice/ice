// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;
using System.Net.Sockets;

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

    public override bool Equals(object obj)
    {
        Connector p = null;

        try
        {
            p = (Connector)obj;
        }
        catch(System.InvalidCastException)
        {
            return false;
        }

        if(this == p)
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

    private IceInternal.Connector _connector;
    private Configuration _configuration;
}
