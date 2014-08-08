// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceWS;

final class ConnectorI implements IceInternal.Connector
{
    @Override
    public IceInternal.Transceiver connect()
    {
        return new TransceiverI(_instance, _delegate.connect(), _host, _port, _resource);
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

    ConnectorI(Instance instance, IceInternal.Connector del, String host, int port, String resource)
    {
        _instance = instance;
        _delegate = del;
        _host = host;
        _port = port;
        _resource = resource;
    }

    @Override
    public boolean equals(java.lang.Object obj)
    {
        if(!(obj instanceof ConnectorI))
        {
            return false;
        }

        if(this == obj)
        {
            return true;
        }

        ConnectorI p = (ConnectorI)obj;
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

    private Instance _instance;
    private IceInternal.Connector _delegate;
    private String _host;
    private int _port;
    private String _resource;
}
