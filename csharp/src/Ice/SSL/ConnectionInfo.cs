// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Security.Cryptography.X509Certificates;

namespace Ice.SSL;

public sealed class ConnectionInfo : Ice.ConnectionInfo
{
    public readonly string cipher;
    public readonly X509Certificate2[] certs;
    public readonly bool verified;

    internal ConnectionInfo(Ice.ConnectionInfo underlying, string cipher, X509Certificate2[] certs, bool verified)
        : base(underlying)
    {
        this.cipher = cipher;
        this.certs = certs;
        this.verified = verified;
    }
}
