// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;

public class CertificateVerifierI : IceSSL.CertificateVerifier
{
    public CertificateVerifierI()
    {
        reset();
    }

    public bool verify(IceSSL.NativeConnectionInfo info)
    {
        hadCert_ = info.nativeCerts != null;
        invoked_ = true;
        return returnValue_;
    }

    internal void reset()
    {
        returnValue_ = true;
        invoked_ = false;
        hadCert_ = false;
    }

    internal void returnValue(bool b)
    {
        returnValue_ = b;
    }

    internal bool invoked()
    {
        return invoked_;
    }

    internal bool hadCert()
    {
        return hadCert_;
    }

    private bool returnValue_;
    private bool invoked_;
    private bool hadCert_;
}
