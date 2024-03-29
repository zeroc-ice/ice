//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

import com.zeroc.Ice.InitializationData;

final class ConnectorI implements com.zeroc.IceInternal.Connector
{
    @Override
    public com.zeroc.IceInternal.Transceiver connect()
    {
        InitializationData initData = _instance.initializationData();
        SSLEngineFactory sslEngineFactory = initData.sslEngineFactory == null ? 
            (peerHost, peerPort) -> _instance.engine().createSSLEngine(false, peerHost, peerPort) : initData.sslEngineFactory;
        return new TransceiverI(
            _instance, 
            _delegate.connect(),
            _host,
            false,
            sslEngineFactory);
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
    ConnectorI(Instance instance, com.zeroc.IceInternal.Connector delegate, String host)
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
    private com.zeroc.IceInternal.Connector _delegate;
    private String _host;
}
