// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class ConnectorI implements IceInternal.Connector
{
    @Override
    public IceInternal.Transceiver connect()
    {
        //
        // The plug-in may not be fully initialized.
        //
        if(!_instance.initialized())
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: plug-in is not initialized";
            throw ex;
        }

        return new TransceiverI(_instance, _delegate.connect(), _host, false);
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

    //
    // Only for use by EndpointI.
    //
    ConnectorI(Instance instance, IceInternal.Connector delegate, String host)
    {
        _instance = instance;
        _delegate = delegate;
        _host = host;
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
        return p._delegate.equals(_delegate);
    }

    private Instance _instance;
    private IceInternal.Connector _delegate;
    private String _host;
}
