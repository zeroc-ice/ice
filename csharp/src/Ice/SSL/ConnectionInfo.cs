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
    /// The certificate chain.
    /// </summary>
    public readonly X509Certificate2[] certs;

    /// <summary>
    /// The certificate chain verification status.
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
