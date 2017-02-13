// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;

import test.IceSSL.configuration.Test.Server;

class ServerI implements Server
{
    ServerI(com.zeroc.Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    @Override
    public void noCert(com.zeroc.Ice.Current current)
    {
        try
        {
            com.zeroc.IceSSL.NativeConnectionInfo info = (com.zeroc.IceSSL.NativeConnectionInfo)current.con.getInfo();
            test(info.certs == null);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
    }

    @Override
    public void
    checkCert(String subjectDN, String issuerDN, com.zeroc.Ice.Current current)
    {
        try
        {
            com.zeroc.IceSSL.NativeConnectionInfo info = (com.zeroc.IceSSL.NativeConnectionInfo)current.con.getInfo();
            java.security.cert.X509Certificate cert = (java.security.cert.X509Certificate)info.nativeCerts[0];
            test(info.verified);
            test(info.nativeCerts.length == 2 &&
                 cert.getSubjectDN().toString().equals(subjectDN) &&
                 cert.getIssuerDN().toString().equals(issuerDN));
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
    }

    @Override
    public void checkCipher(String cipher, com.zeroc.Ice.Current current)
    {
        try
        {
            com.zeroc.IceSSL.NativeConnectionInfo info = (com.zeroc.IceSSL.NativeConnectionInfo)current.con.getInfo();
            test(info.cipher.indexOf(cipher) >= 0);
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            test(false);
        }
    }

    public void destroy()
    {
        _communicator.destroy();
    }

    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private com.zeroc.Ice.Communicator _communicator;
}
