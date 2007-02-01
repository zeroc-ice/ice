// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

class ServerI extends Test._ServerDisp
{
    ServerI(Ice.Communicator communicator)
    {
        _communicator = communicator;
    }

    public void
    noCert(Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = IceSSL.Util.getConnectionInfo(current.con);
            test(info.certs == null);
        }
        catch(IceSSL.ConnectionInvalidException ex)
        {
            test(false);
        }
    }

    public void
    checkCert(String subjectDN, String issuerDN, Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = IceSSL.Util.getConnectionInfo(current.con);
            java.security.cert.X509Certificate cert = (java.security.cert.X509Certificate)info.certs[0];
            test(info.certs.length == 2 &&
                 cert.getSubjectDN().toString().equals(subjectDN) &&
                 cert.getIssuerDN().toString().equals(issuerDN));
        }
        catch(IceSSL.ConnectionInvalidException ex)
        {
            test(false);
        }
    }

    public void
    checkCipher(String cipher, Ice.Current current)
    {
        try
        {
            IceSSL.ConnectionInfo info = IceSSL.Util.getConnectionInfo(current.con);
            test(info.cipher.indexOf(cipher) >= 0);
        }
        catch(IceSSL.ConnectionInvalidException ex)
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
