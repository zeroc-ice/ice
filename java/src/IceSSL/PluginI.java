// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class PluginI implements Plugin
{
    public PluginI(Ice.Communicator communicator)
    {
        IceInternal.ProtocolPluginFacade facade = IceInternal.Util.getProtocolPluginFacade(communicator);

        _engine = new SSLEngine(facade);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //
        facade.addEndpointFactory(
            new EndpointFactoryI(new Instance(_engine, IceSSL.EndpointType.value, "ssl")));
    }

    public void initialize()
    {
        _engine.initialize();
    }

    public void destroy()
    {
    }

    public void setContext(javax.net.ssl.SSLContext context)
    {
        _engine.context(context);
    }

    public javax.net.ssl.SSLContext getContext()
    {
        return _engine.context();
    }

    public void setCertificateVerifier(CertificateVerifier verifier)
    {
        _engine.setCertificateVerifier(verifier);
    }

    public CertificateVerifier getCertificateVerifier()
    {
        return _engine.getCertificateVerifier();
    }

    public void setPasswordCallback(PasswordCallback callback)
    {
        _engine.setPasswordCallback(callback);
    }

    public PasswordCallback getPasswordCallback()
    {
        return _engine.getPasswordCallback();
    }

    public void setKeystoreStream(java.io.InputStream stream)
    {
        _engine.setKeystoreStream(stream);
    }

    public void setTruststoreStream(java.io.InputStream stream)
    {
        _engine.setTruststoreStream(stream);
    }

    public void addSeedStream(java.io.InputStream stream)
    {
        _engine.addSeedStream(stream);
    }

    private SSLEngine _engine;
}
