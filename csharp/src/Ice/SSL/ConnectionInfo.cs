// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Security.Cryptography.X509Certificates;

namespace Ice.SSL;

/// <summary>
/// Provides access to the connection details of an SSL connection.
/// </summary>
public sealed class ConnectionInfo : Ice.ConnectionInfo
{
    /// <summary>
    /// The negotiated cipher suite.
    /// </summary>
    public readonly string cipher;

    /// <summary>
    /// The peer's certificate, in a single-element array; empty when the peer did not provide a certificate.
    /// </summary>
    public readonly X509Certificate2[] certs;

    /// <summary>
    /// The certificate verification status. This field is always <see langword="true" />: a connection whose
    /// certificate verification fails is aborted during the SSL handshake.
    /// </summary>
    public readonly bool verified;

    internal ConnectionInfo(Ice.ConnectionInfo underlying, string cipher, X509Certificate2[] certs, bool verified)
        : base(underlying)
    {
        this.cipher = cipher;
        this.certs = certs;
        this.verified = verified;
    }
}
