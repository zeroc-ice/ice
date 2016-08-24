// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.classLoader;

public class CertificateVerifierI implements com.zeroc.IceSSL.CertificateVerifier
{
    @Override
    public boolean verify(com.zeroc.IceSSL.NativeConnectionInfo info)
    {
        return true;
    }
}
