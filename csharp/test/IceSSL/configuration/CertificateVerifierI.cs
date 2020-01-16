//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

public class CertificateVerifier : IceSSL.ICertificateVerifier
{
    public CertificateVerifier() => reset();

    public bool verify(IceSSL.ConnectionInfo info)
    {
        _hadCert = info.Certs != null;
        _invoked = true;
        return _returnValue;
    }

    internal void reset()
    {
        _returnValue = true;
        _invoked = false;
        _hadCert = false;
    }

    internal void returnValue(bool b) => _returnValue = b;

    internal bool invoked() => _invoked;

    internal bool hadCert() => _hadCert;

    private bool _returnValue;
    private bool _invoked;
    private bool _hadCert;
}
