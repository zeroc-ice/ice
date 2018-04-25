// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

final class AcceptorI implements IceInternal.Acceptor
{
    @Override
    public java.nio.channels.ServerSocketChannel fd()
    {
        return _delegate.fd();
    }

    @Override
    public void setReadyCallback(IceInternal.ReadyCallback callback)
    {
        _delegate.setReadyCallback(callback);
    }

    @Override
    public void close()
    {
        _delegate.close();
    }

    @Override
    public IceInternal.EndpointI listen()
    {
        _endpoint = _endpoint.endpoint(_delegate.listen());
        return _endpoint;
    }

    @Override
    public IceInternal.Transceiver accept()
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

        return new TransceiverI(_instance, _delegate.accept(), _adapterName, true);
    }

    @Override
    public String protocol()
    {
        return _delegate.protocol();
    }

    @Override
    public String toString()
    {
        return _delegate.toString();
    }

    @Override
    public String toDetailedString()
    {
        return _delegate.toDetailedString();
    }

    AcceptorI(EndpointI endpoint, Instance instance, IceInternal.Acceptor delegate, String adapterName)
    {
        _endpoint = endpoint;
        _instance = instance;
        _delegate = delegate;
        _adapterName = adapterName;
    }

    private EndpointI _endpoint;
    private Instance _instance;
    private IceInternal.Acceptor _delegate;
    private String _adapterName;
}
