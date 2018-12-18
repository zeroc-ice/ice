// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.classLoader;

public class CertificateVerifierI implements IceSSL.CertificateVerifier
{
    @Override
    public boolean verify(IceSSL.ConnectionInfo info)
    {
        return true;
    }
}
