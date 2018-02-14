// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;
import test.IceSSL.configuration.Test._ServerDisp;

class ServerI extends _ServerDisp
{
    ServerI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void
    noCert(Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.con.getInfo();
            test(info.certs == null);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
    }

    @Override
    public void
    checkCert(String subjectDN, String issuerDN, Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.con.getInfo();
            java.security.cert.X509Certificate cert = (java.security.cert.X509Certificate)info.certs[0];
            test(info.verified);
            test(info.certs.length == 2 &&
                 cert.getSubjectDN().toString().equals(subjectDN) &&
                 cert.getIssuerDN().toString().equals(issuerDN));
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
    }

    @Override
    public void
    checkCipher(String cipher, Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)current.con.getInfo();
            test(info.cipher.indexOf(cipher) >= 0);
        }
        catch(Ice.LocalException ex)
        {
            test(false);
        }
    }

    public void
    destroy()
    {
        _communicator.destroy();
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private Ice.Communicator _communicator;
}
