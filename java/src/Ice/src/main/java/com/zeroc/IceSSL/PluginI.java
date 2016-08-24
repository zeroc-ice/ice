// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceSSL;

class PluginI implements Plugin
{
    public PluginI(com.zeroc.Ice.Communicator communicator)
    {
        final com.zeroc.IceInternal.ProtocolPluginFacade facade =
            com.zeroc.IceInternal.Util.getProtocolPluginFacade(communicator);
        _engine = new SSLEngine(facade);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //

        // SSL based on TCP
        com.zeroc.IceInternal.EndpointFactory tcp = facade.getEndpointFactory(com.zeroc.Ice.TCPEndpointType.value);
        if(tcp != null)
        {
            Instance instance = new Instance(_engine, com.zeroc.Ice.SSLEndpointType.value, "ssl");
            facade.addEndpointFactory(new EndpointFactoryI(instance, tcp.clone(instance, null)));
        }

        // SSL based on Bluetooth
        com.zeroc.IceInternal.EndpointFactory bluetooth = facade.getEndpointFactory(com.zeroc.Ice.BTEndpointType.value);
        if(bluetooth != null)
        {
            Instance instance = new Instance(_engine, com.zeroc.Ice.BTSEndpointType.value, "bts");
            facade.addEndpointFactory(new EndpointFactoryI(instance, bluetooth.clone(instance, null)));
        }
    }

    @Override
    public void initialize()
    {
        _engine.initialize();
    }

    @Override
    public void destroy()
    {
    }

    @Override
    public void setContext(javax.net.ssl.SSLContext context)
    {
        _engine.context(context);
    }

    @Override
    public javax.net.ssl.SSLContext getContext()
    {
        return _engine.context();
    }

    @Override
    public void setCertificateVerifier(CertificateVerifier verifier)
    {
        _engine.setCertificateVerifier(verifier);
    }

    @Override
    public CertificateVerifier getCertificateVerifier()
    {
        return _engine.getCertificateVerifier();
    }

    @Override
    public void setPasswordCallback(PasswordCallback callback)
    {
        _engine.setPasswordCallback(callback);
    }

    @Override
    public PasswordCallback getPasswordCallback()
    {
        return _engine.getPasswordCallback();
    }

    @Override
    public void setKeystoreStream(java.io.InputStream stream)
    {
        _engine.setKeystoreStream(stream);
    }

    @Override
    public void setTruststoreStream(java.io.InputStream stream)
    {
        _engine.setTruststoreStream(stream);
    }

    @Override
    public void addSeedStream(java.io.InputStream stream)
    {
        _engine.addSeedStream(stream);
    }

    private SSLEngine _engine;
}
