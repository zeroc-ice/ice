// Copyright (c) ZeroC, Inc.

#nullable enable

using System.Security.Cryptography.X509Certificates;

namespace Ice.SSL;

public sealed class ConnectionInfo : Ice.ConnectionInfo
{
    public string cipher = "";
    public X509Certificate2[] certs = [];
    public bool verified;
}
