// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class PluginI extends Ice.LocalObjectImpl implements Plugin
{
    public
    PluginI(Ice.Communicator communicator)
    {
        _instance = new Instance(communicator);
    }

    public void
    initialize()
    {
        _instance.initialize();
    }

    public void
    destroy()
    {
    }

    public void
    setContext(javax.net.ssl.SSLContext context)
    {
        _instance.context(context);
    }

    public javax.net.ssl.SSLContext
    getContext()
    {
        return _instance.context();
    }

    public void
    setCertificateVerifier(CertificateVerifier verifier)
    {
        _instance.setCertificateVerifier(verifier);
    }

    private Instance _instance;
}
