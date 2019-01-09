// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package IceInternal;

final class WSConnector implements Connector
{
    @Override
    public Transceiver connect()
    {
        return new WSTransceiver(_instance, _delegate.connect(), _host, _resource);
    }

    @Override
    public short type()
    {
        return _delegate.type();
    }

    @Override
    public String toString()
    {
        return _delegate.toString();
    }

    @Override
    public int hashCode()
    {
        return _delegate.hashCode();
    }

    WSConnector(ProtocolInstance instance, Connector del, String host, String resource)
    {
        _instance = instance;
        _delegate = del;
        _host = host;
        _resource = resource;
    }

    @Override
    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof WSConnector))
        {
            return false;
        }

        if(this == obj)
        {
            return true;
        }

        WSConnector p = (WSConnector)obj;
        if(!_delegate.equals(p._delegate))
        {
            return false;
        }

        if(!_resource.equals(p._resource))
        {
            return false;
        }

        return true;
    }

    private ProtocolInstance _instance;
    private Connector _delegate;
    private String _host;
    private String _resource;
}
