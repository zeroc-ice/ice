// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Net.Security;
using System.Security;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;
using System.Text;
using Ice.Internal;

namespace Ice.SSL;

internal class SSLEngine
{
    internal SSLEngine(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _logger = communicator.getLogger();
        _securityTraceLevel = _communicator.getProperties().getIcePropertyAsInt("IceSSL.Trace.Security");
        _securityTraceCategory = "Security";
        _trustManager = new TrustManager(_communicator);
    }

    internal void initialize()
    {
        Ice.Properties properties = communicator().getProperties();

        // Check for a default directory. We look in this directory for files mentioned in the configuration.
        _defaultDir = properties.getIceProperty("IceSSL.DefaultDir");

        _verifyPeer = properties.getIcePropertyAsInt("IceSSL.VerifyPeer");

        // CheckCRL determines whether the certificate revocation list is checked, and how strictly.
        _checkCRL = properties.getIcePropertyAsInt("IceSSL.CheckCRL");

        string certStoreLocation = properties.getIceProperty("IceSSL.CertStoreLocation");
        StoreLocation storeLocation;
        if (certStoreLocation == "CurrentUser")
        {
            storeLocation = StoreLocation.CurrentUser;
        }
        else if (certStoreLocation == "LocalMachine")
        {
            storeLocation = StoreLocation.LocalMachine;
        }
        else
        {
            _logger.warning(
                "Invalid IceSSL.CertStoreLocation value `" + certStoreLocation + "' adjusted to `CurrentUser'");
            storeLocation = StoreLocation.CurrentUser;
        }
        _useMachineContext = certStoreLocation == "LocalMachine";

        // CheckCertName determines whether we compare the name in a peer's certificate against its hostname.
        _checkCertName = properties.getIcePropertyAsInt("IceSSL.CheckCertName") > 0;

        Debug.Assert(_certs == null);
        // If IceSSL.CertFile is defined, load a certificate from a file and add it to the collection.
        _certs = [];
        string certFile = properties.getIceProperty("IceSSL.CertFile");
        string passwordStr = properties.getIceProperty("IceSSL.Password");
        string findCert = properties.getIceProperty("IceSSL.FindCert");

        if (certFile.Length > 0)
        {
            if (!checkPath(ref certFile))
            {
                throw new Ice.InitializationException($"IceSSL: certificate file not found: {certFile}");
            }

            try
            {
                X509Certificate2 cert;
                X509KeyStorageFlags importFlags;
                if (_useMachineContext)
                {
                    importFlags = X509KeyStorageFlags.MachineKeySet;
                }
                else
                {
                    importFlags = X509KeyStorageFlags.UserKeySet;
                }

                if (passwordStr.Length > 0)
                {
                    using SecureString password = createSecureString(passwordStr);
                    cert = new X509Certificate2(certFile, password, importFlags);
                }
                else
                {
                    cert = new X509Certificate2(certFile, (string)null, importFlags);
                }
                _certs.Add(cert);
            }
            catch (CryptographicException ex)
            {
                throw new Ice.InitializationException(
                    $"IceSSL: error while attempting to load certificate from {certFile}",
                    ex);
            }
        }
        else if (findCert.Length > 0)
        {
            string certStore = properties.getIceProperty("IceSSL.CertStore");
            _certs.AddRange(findCertificates("IceSSL.FindCert", storeLocation, certStore, findCert));
            if (_certs.Count == 0)
            {
                throw new Ice.InitializationException("IceSSL: no certificates found");
            }
        }

        Debug.Assert(_caCerts == null);
        string certAuthFile = properties.getIceProperty("IceSSL.CAs");
        if (certAuthFile.Length > 0 || properties.getIcePropertyAsInt("IceSSL.UsePlatformCAs") <= 0)
        {
            _caCerts = [];
        }

        if (certAuthFile.Length > 0)
        {
            if (!checkPath(ref certAuthFile))
            {
                throw new Ice.InitializationException($"IceSSL: CA certificate file not found: {certAuthFile}");
            }
            try
            {
                try
                {
                    // First try to import as a PEM file, which supports importing multiple certificates from a PEM
                    // encoded file
                    _caCerts.ImportFromPemFile(certAuthFile);
                }
                catch (CryptographicException)
                {
                    // Expected if the file is not in PEM format.
                }

                if (_caCerts.Count == 0)
                {
                    // Fallback to Import which handles DER/PFX.
                    _caCerts.Import(certAuthFile);
                }
            }
            catch (Exception ex)
            {
                throw new Ice.InitializationException(
                    $"IceSSL: error while attempting to load CA certificate from {certAuthFile}",
                    ex);
            }
        }
    }

    internal bool useMachineContext() => _useMachineContext;

    internal X509Certificate2Collection caCerts() => _caCerts;

    internal Ice.Communicator communicator() => _communicator;

    internal int securityTraceLevel() => _securityTraceLevel;

    internal string securityTraceCategory() => _securityTraceCategory;

    internal X509Certificate2Collection certs() => _certs;

    internal void traceStream(SslStream stream, string connInfo)
    {
        var s = new StringBuilder();
        s.Append("SSL connection summary");
        if (connInfo.Length > 0)
        {
            s.Append('\n');
            s.Append(connInfo);
        }
        s.Append("\nauthenticated = " + (stream.IsAuthenticated ? "yes" : "no"));
        s.Append("\nencrypted = " + (stream.IsEncrypted ? "yes" : "no"));
        s.Append("\nsigned = " + (stream.IsSigned ? "yes" : "no"));
        s.Append("\nmutually authenticated = " + (stream.IsMutuallyAuthenticated ? "yes" : "no"));
        s.Append("\nhash algorithm = " + stream.HashAlgorithm + "/" + stream.HashStrength);
        s.Append("\ncipher algorithm = " + stream.CipherAlgorithm + "/" + stream.CipherStrength);
        s.Append("\nkey exchange algorithm = " + stream.KeyExchangeAlgorithm + "/" + stream.KeyExchangeStrength);
        s.Append("\nprotocol = " + stream.SslProtocol);
        _logger.trace(_securityTraceCategory, s.ToString());
    }

    internal void verifyPeer(ConnectionInfo info, string description)
    {
        if (!_trustManager.verify(info, description))
        {
            string msg = (info.incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                description;
            if (_securityTraceLevel >= 1)
            {
                _logger.trace(_securityTraceCategory, msg);
            }

            throw new SecurityException($"IceSSL: {msg}");
        }
    }

    internal SslClientAuthenticationOptions createClientAuthenticationOptions(
        RemoteCertificateValidationCallback remoteCertificateValidationCallback,
        string host)
    {
        var authenticationOptions = new SslClientAuthenticationOptions
        {
            ClientCertificates = _certs,
            LocalCertificateSelectionCallback = (sender, targetHost, certs, remoteCertificate, acceptableIssuers) =>
            {
                if (certs == null || certs.Count == 0)
                {
                    return null;
                }
                else if (certs.Count == 1)
                {
                    return certs[0];
                }

                // Use the first certificate that match the acceptable issuers.
                if (acceptableIssuers != null && acceptableIssuers.Length > 0)
                {
                    foreach (X509Certificate certificate in certs)
                    {
                        if (Array.IndexOf(acceptableIssuers, certificate.Issuer) != -1)
                        {
                            return certificate;
                        }
                    }
                }
                return certs[0];
            },
            RemoteCertificateValidationCallback = remoteCertificateValidationCallback,
            TargetHost = host,
        };

        authenticationOptions.CertificateChainPolicy = new X509ChainPolicy();
        if (_caCerts is null)
        {
            authenticationOptions.CertificateChainPolicy.TrustMode = X509ChainTrustMode.System;
        }
        else
        {
            authenticationOptions.CertificateChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;
            foreach (X509Certificate certificate in _caCerts)
            {
                authenticationOptions.CertificateChainPolicy.CustomTrustStore.Add(certificate);
            }
        }

        if (!_checkCertName)
        {
            authenticationOptions.CertificateChainPolicy.VerificationFlags |= X509VerificationFlags.IgnoreInvalidName;
        }

        if (_checkCRL == 1)
        {
            authenticationOptions.CertificateChainPolicy.VerificationFlags |= X509VerificationFlags.IgnoreCertificateAuthorityRevocationUnknown;
        }
        authenticationOptions.CertificateChainPolicy.RevocationMode =
            _checkCRL == 0 ? X509RevocationMode.NoCheck : X509RevocationMode.Online;
        return authenticationOptions;
    }

    internal SslServerAuthenticationOptions createServerAuthenticationOptions(
        RemoteCertificateValidationCallback remoteCertificateValidationCallback)
    {
        // Get the certificate collection and select the first one.
        X509Certificate2 cert = null;
        if (_certs.Count > 0)
        {
            cert = _certs[0];
        }

        var authenticationOptions = new SslServerAuthenticationOptions
        {
            ServerCertificate = cert,
            ClientCertificateRequired = _verifyPeer > 0,
            RemoteCertificateValidationCallback = remoteCertificateValidationCallback,
            CertificateRevocationCheckMode = X509RevocationMode.NoCheck
        };

        authenticationOptions.CertificateChainPolicy = new X509ChainPolicy();
        if (_caCerts is null)
        {
            authenticationOptions.CertificateChainPolicy.TrustMode = X509ChainTrustMode.System;
        }
        else
        {
            authenticationOptions.CertificateChainPolicy.TrustMode = X509ChainTrustMode.CustomRootTrust;
            foreach (X509Certificate certificate in _caCerts)
            {
                authenticationOptions.CertificateChainPolicy.CustomTrustStore.Add(certificate);
            }
        }
        authenticationOptions.CertificateChainPolicy.RevocationMode =
            _checkCRL == 0 ? X509RevocationMode.NoCheck : X509RevocationMode.Online;
        if (_checkCRL == 1)
        {
            authenticationOptions.CertificateChainPolicy.VerificationFlags |= X509VerificationFlags.IgnoreCertificateAuthorityRevocationUnknown;
        }
        return authenticationOptions;
    }

    private static bool isAbsolutePath(string path)
    {
        // Skip whitespace
        path = path.Trim();

        if (AssemblyUtil.isWindows)
        {
            // We need at least 3 non-whitespace characters to have an absolute path
            if (path.Length < 3)
            {
                return false;
            }

            // Check for X:\ path ('\' may have been converted to '/')
            if ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))
            {
                return path[1] == ':' && (path[2] == '\\' || path[2] == '/');
            }
        }
        // Check for UNC path
        return (path[0] == '\\' && path[1] == '\\') || path[0] == '/';
    }

    private static X509Certificate2Collection findCertificates(
        string prop,
        StoreLocation storeLocation,
        string name,
        string value)
    {
        // Open the X509 certificate store.
        X509Store store;
        try
        {
            try
            {
                store = new X509Store((StoreName)Enum.Parse(typeof(StoreName), name, true), storeLocation);
            }
            catch (ArgumentException)
            {
                store = new X509Store(name, storeLocation);
            }
            store.Open(OpenFlags.ReadOnly);
        }
        catch (Exception ex)
        {
            throw new Ice.InitializationException($"IceSSL: failure while opening store specified by {prop}", ex);
        }

        // Start with all of the certificates in the collection and filter as necessary.
        //
        // - If the value is "*", return all certificates.
        // - Otherwise, search using key:value pairs. The following keys are supported:
        //
        //   Issuer
        //   IssuerDN
        //   Serial
        //   Subject
        //   SubjectDN
        //   SubjectKeyId
        //   Thumbprint
        //
        //   A value must be enclosed in single or double quotes if it contains whitespace.
        X509Certificate2Collection result = [.. store.Certificates];
        try
        {
            if (value != "*")
            {
                if (!value.Contains(':', StringComparison.Ordinal))
                {
                    throw new Ice.InitializationException($"IceSSL: no key in `{value}'");
                }
                int start = 0;
                int pos;
                while ((pos = value.IndexOf(':', start)) != -1)
                {
                    // Parse the X509FindType.
                    string field = value[start..pos].Trim().ToUpperInvariant();
                    X509FindType findType;
                    if (field == "SUBJECT")
                    {
                        findType = X509FindType.FindBySubjectName;
                    }
                    else if (field == "SUBJECTDN")
                    {
                        findType = X509FindType.FindBySubjectDistinguishedName;
                    }
                    else if (field == "ISSUER")
                    {
                        findType = X509FindType.FindByIssuerName;
                    }
                    else if (field == "ISSUERDN")
                    {
                        findType = X509FindType.FindByIssuerDistinguishedName;
                    }
                    else if (field == "THUMBPRINT")
                    {
                        findType = X509FindType.FindByThumbprint;
                    }
                    else if (field == "SUBJECTKEYID")
                    {
                        findType = X509FindType.FindBySubjectKeyIdentifier;
                    }
                    else if (field == "SERIAL")
                    {
                        findType = X509FindType.FindBySerialNumber;
                    }
                    else
                    {
                        throw new Ice.InitializationException($"IceSSL: unknown key in `{value}'");
                    }

                    // Parse the argument.
                    start = pos + 1;
                    while (start < value.Length && (value[start] == ' ' || value[start] == '\t'))
                    {
                        ++start;
                    }

                    if (start == value.Length)
                    {
                        throw new Ice.InitializationException($"IceSSL: missing argument in `{value}'");
                    }

                    string arg;
                    if (value[start] == '"' || value[start] == '\'')
                    {
                        int end = start;
                        ++end;
                        while (end < value.Length)
                        {
                            if (value[end] == value[start] && value[end - 1] != '\\')
                            {
                                break;
                            }
                            ++end;
                        }
                        if (end == value.Length || value[end] != value[start])
                        {
                            throw new Ice.InitializationException($"IceSSL: unmatched quote in `{value}'");
                        }
                        ++start;
                        arg = value[start..end];
                        start = end + 1;
                    }
                    else
                    {
                        char[] ws = [' ', '\t'];
                        int end = value.IndexOfAny(ws, start);
                        if (end == -1)
                        {
                            arg = value[start..];
                            start = value.Length;
                        }
                        else
                        {
                            arg = value[start..end];
                            start = end + 1;
                        }
                    }

                    // Execute the query.
                    //
                    // TODO: allow user to specify a value for validOnly?
                    bool validOnly = false;
                    if (findType == X509FindType.FindBySubjectDistinguishedName ||
                        findType == X509FindType.FindByIssuerDistinguishedName)
                    {
                        X500DistinguishedNameFlags[] flags = [
                            X500DistinguishedNameFlags.None,
                            X500DistinguishedNameFlags.Reversed,
                        ];
                        var dn = new X500DistinguishedName(arg);
                        X509Certificate2Collection r = result;
                        for (int i = 0; i < flags.Length; ++i)
                        {
                            r = result.Find(findType, dn.Decode(flags[i]), validOnly);
                            if (r.Count > 0)
                            {
                                break;
                            }
                        }
                        result = r;
                    }
                    else
                    {
                        result = result.Find(findType, arg, validOnly);
                    }
                }
            }
        }
        finally
        {
            store.Close();
        }

        return result;
    }

    private static SecureString createSecureString(string s)
    {
        var result = new SecureString();
        foreach (char ch in s)
        {
            result.AppendChar(ch);
        }
        return result;
    }

    private bool checkPath(ref string path)
    {
        if (File.Exists(path))
        {
            return true;
        }

        if (_defaultDir.Length > 0 && !isAbsolutePath(path))
        {
            string s = _defaultDir + Path.DirectorySeparatorChar + path;
            if (File.Exists(s))
            {
                path = s;
                return true;
            }
        }
        return false;
    }

    private readonly Ice.Communicator _communicator;
    private readonly Ice.Logger _logger;
    private readonly int _securityTraceLevel;
    private readonly string _securityTraceCategory;
    private string _defaultDir;
    private bool _checkCertName;
    private int _verifyPeer;
    private int _checkCRL;
    private X509Certificate2Collection _certs;
    private bool _useMachineContext;
    private X509Certificate2Collection _caCerts;
    private readonly TrustManager _trustManager;
}
