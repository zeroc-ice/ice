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

        _sharedInstance = new SharedInstance(facade);

        //
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plug-in is fully initialized.
        //
        facade.addEndpointFactory(
            new EndpointFactoryI(new Instance(_sharedInstance, IceSSL.EndpointType.value, "ssl")));
    }

    public void initialize()
    {
        _sharedInstance.initialize();
    }

    public void destroy()
    {
    }

    public void setContext(javax.net.ssl.SSLContext context)
    {
        _sharedInstance.context(context);
    }

    public javax.net.ssl.SSLContext getContext()
    {
        return _sharedInstance.context();
    }

    public void setCertificateVerifier(CertificateVerifier verifier)
    {
        _sharedInstance.setCertificateVerifier(verifier);
    }

    public CertificateVerifier getCertificateVerifier()
    {
        return _sharedInstance.getCertificateVerifier();
    }

    public void setPasswordCallback(PasswordCallback callback)
    {
        _sharedInstance.setPasswordCallback(callback);
    }

    public PasswordCallback getPasswordCallback()
    {
        return _sharedInstance.getPasswordCallback();
    }

    public void setKeystoreStream(java.io.InputStream stream)
    {
        _sharedInstance.setKeystoreStream(stream);
    }

    public void setTruststoreStream(java.io.InputStream stream)
    {
        _sharedInstance.setTruststoreStream(stream);
    }

    public void addSeedStream(java.io.InputStream stream)
    {
        _sharedInstance.addSeedStream(stream);
    }

    private SharedInstance _sharedInstance;
}
