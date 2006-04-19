// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    destroy()
    {
    }

    public void
    initialize(javax.net.ssl.SSLContext context)
    {
	_instance.initialize(context);
    }

    public void
    setCertificateVerifier(CertificateVerifier verifier)
    {
	_instance.setCertificateVerifier(verifier);
    }

    private Instance _instance;
}
