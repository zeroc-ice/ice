// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
        _hadCert = info.nativeCerts != null;
        _invoked = true;
        return _returnValue;
    }

    internal void reset()
    {
        _returnValue = true;
        _invoked = false;
        _hadCert = false;
    }

    internal void returnValue(bool b)
    {
        _returnValue = b;
    }

    internal bool invoked()
    {
        return _invoked;
    }

    internal bool hadCert()
    {
        return _hadCert;
    }

    private bool _returnValue;
    private bool _invoked;
    private bool _hadCert;
}
