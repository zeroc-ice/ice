//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceSSL;

import com.zeroc.IceInternal.ProtocolPluginFacade;

class PluginI implements Plugin
{
    public PluginI(com.zeroc.Ice.Communicator communicator)
    {
        final ProtocolPluginFacade facade = com.zeroc.IceInternal.Util.getProtocolPluginFacade(communicator);
        _engine = new SSLEngine(facade);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //
        Instance instance = new Instance(_engine, com.zeroc.Ice.SSLEndpointType.value, "ssl");
        facade.addEndpointFactory(new EndpointFactoryI(instance, com.zeroc.Ice.TCPEndpointType.value));
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
