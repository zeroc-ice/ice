//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class CertificateVerifierI : IceSSL.CertificateVerifier
{
    public CertificateVerifierI()
    {
        reset();
    }

    public bool verify(IceSSL.ConnectionInfo info)
    {
        _hadCert = info.certs != null;
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
