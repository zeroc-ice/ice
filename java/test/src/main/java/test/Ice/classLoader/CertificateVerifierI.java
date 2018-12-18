// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.classLoader;

public class CertificateVerifierI implements com.zeroc.IceSSL.CertificateVerifier
{
    @Override
    public boolean verify(com.zeroc.IceSSL.ConnectionInfo info)
    {
        return true;
    }
}
