//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using System.IO;
using System.Net.Security;
using System.Security;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;

namespace ZeroC.Ice
{
    /// <summary>The IPasswordCallback delegate provides applications a way of supplying the SSL transport with
    /// passwords; this avoids using plain text configuration properties. Obtain the password necessary to access
    /// the private key associated with the certificate in the given file.</summary>
    /// <param name="file">The certificate file name.</param>
    /// <returns>The password for the key or null, if no password is necessary.</returns>
    public delegate SecureString IPasswordCallback(string file);

    internal class SslEngine
    {
        internal X509Certificate2Collection? CaCerts { get; }
        internal X509Certificate2Collection? Certs { get; }
        internal RemoteCertificateValidationCallback? RemoteCertificateValidationCallback { get; }
        internal bool CheckCertName { get; }
        internal int CheckCRL { get; }
        internal IPasswordCallback? PasswordCallback { get; }
        internal int SecurityTraceLevel { get; }
        internal string SecurityTraceCategory => "Security";

        internal SslProtocols SslProtocols { get; }
        internal bool UseMachineContext { get; }

        private readonly string _defaultDir = string.Empty;
        private readonly ILogger _logger;
        private readonly SslTrustManager _trustManager;
        private readonly int _verifyDepthMax;

        internal SslEngine(
            Communicator communicator,
            X509Certificate2Collection? certificates,
            X509Certificate2Collection? caCertificates,
            RemoteCertificateValidationCallback? certificateValidationCallback,
            IPasswordCallback? passwordCallback)
        {
            _logger = communicator.Logger;
            SecurityTraceLevel = communicator.GetPropertyAsInt("IceSSL.Trace.Security") ?? 0;
            _trustManager = new SslTrustManager(communicator);

            RemoteCertificateValidationCallback = certificateValidationCallback;
            PasswordCallback = passwordCallback;

            Certs = certificates;
            CaCerts = caCertificates;

            // Check for a default directory. We look in this directory for files mentioned in the configuration.
            _defaultDir = communicator.GetProperty("IceSSL.DefaultDir") ?? "";

            string certStoreLocation = communicator.GetProperty("IceSSL.CertStoreLocation") ?? "CurrentUser";
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
                _logger.Warning($"Invalid IceSSL.CertStoreLocation value `{certStoreLocation}' adjusted to `CurrentUser'");
                storeLocation = StoreLocation.CurrentUser;
            }
            UseMachineContext = certStoreLocation == "LocalMachine";

            // Protocols selects which protocols to enable
            SslProtocols = ParseProtocols(communicator.GetPropertyAsList("IceSSL.Protocols"));

            // CheckCertName determines whether we compare the name in a peer's certificate against its hostname.
            bool? checkCertName = communicator.GetPropertyAsBool("IceSSL.CheckCertName");
            if (checkCertName != null && RemoteCertificateValidationCallback != null)
            {
                throw new InvalidConfigurationException(
                    "the property `IceSSL.CheckCertName' check is incompatible with the custom remote certificate validation callback");
            }
            CheckCertName = checkCertName ?? false;

            // VerifyDepthMax establishes the maximum length of a peer's certificate chain, including the peer's
            // certificate. A value of 0 means there is no maximum.
            int? verifyDepthMax = communicator.GetPropertyAsInt("IceSSL.VerifyDepthMax");
            if (verifyDepthMax != null && RemoteCertificateValidationCallback != null)
            {
                throw new InvalidConfigurationException(
                    "the property `IceSSL.VerifyDepthMax' check is incompatible with the custom remote certificate validation callback");
            }
            _verifyDepthMax = verifyDepthMax ?? 3;

            // CheckCRL determines whether the certificate revocation list is checked, and how strictly.
            CheckCRL = communicator.GetPropertyAsInt("IceSSL.CheckCRL") ?? 0;

            // If the user hasn't supplied a certificate collection, we need to examine the property settings.
            if (Certs == null)
            {
                // If IceSSL.CertFile is defined, load a certificate from a file and add it to the collection.
                // TODO: tracing?
                Certs = new X509Certificate2Collection();
                string? certFile = communicator.GetProperty("IceSSL.CertFile");
                string? passwordStr = communicator.GetProperty("IceSSL.Password");
                string? findCert = communicator.GetProperty("IceSSL.FindCert");

                if (certFile != null)
                {
                    if (!CheckPath(ref certFile))
                    {
                        throw new FileNotFoundException($"certificate file not found: `{certFile}'", certFile);
                    }

                    SecureString? password = null;
                    if (passwordStr != null)
                    {
                        password = CreateSecureString(passwordStr);
                    }
                    else if (PasswordCallback != null)
                    {
                        password = PasswordCallback(certFile);
                    }

                    try
                    {
                        X509Certificate2 cert;
                        X509KeyStorageFlags importFlags;
                        if (UseMachineContext)
                        {
                            importFlags = X509KeyStorageFlags.MachineKeySet;
                        }
                        else
                        {
                            importFlags = X509KeyStorageFlags.UserKeySet;
                        }

                        if (password != null)
                        {
                            cert = new X509Certificate2(certFile, password, importFlags);
                        }
                        else
                        {
                            cert = new X509Certificate2(certFile, "", importFlags);
                        }
                        Certs.Add(cert);
                    }
                    catch (CryptographicException ex)
                    {
                        throw new InvalidConfigurationException(
                            $"error while attempting to load certificate from `{certFile}'", ex);
                    }
                }
                else if (findCert != null)
                {
                    string certStore = communicator.GetProperty("IceSSL.CertStore") ?? "My";
                    Certs.AddRange(FindCertificates("IceSSL.FindCert", storeLocation, certStore, findCert));
                    if (Certs.Count == 0)
                    {
                        throw new InvalidConfigurationException("no certificates found");
                    }
                }
            }

            if (CaCerts == null)
            {
                string? certAuthFile = communicator.GetProperty("IceSSL.CAs");
                if (certAuthFile != null && RemoteCertificateValidationCallback != null)
                {
                    throw new InvalidConfigurationException(
                        "the property `IceSSL.CAs' is incompatible with the custom remote certificate validation callback");
                }

                bool? usePlatformCAs = communicator.GetPropertyAsBool("IceSSL.UsePlatformCAs");
                if (usePlatformCAs != null && RemoteCertificateValidationCallback != null)
                {
                    throw new InvalidConfigurationException(
                        "the property `IceSSL.UsePlatformCAs' is incompatible with the custom remote certificate validation callback");
                }

                if (RemoteCertificateValidationCallback == null)
                {
                    if (certAuthFile != null || !(usePlatformCAs ?? false))
                    {
                        CaCerts = new X509Certificate2Collection();
                    }

                    if (certAuthFile != null)
                    {
                        if (!CheckPath(ref certAuthFile))
                        {
                            throw new FileNotFoundException("CA certificate file not found: `{certAuthFile}'",
                                certAuthFile);
                        }

                        try
                        {
                            using FileStream fs = File.OpenRead(certAuthFile);
                            byte[] data = new byte[fs.Length];
                            fs.Read(data, 0, data.Length);

                            string strbuf = "";
                            try
                            {
                                strbuf = System.Text.Encoding.UTF8.GetString(data);
                            }
                            catch (Exception)
                            {
                                // Ignore
                            }

                            if (strbuf.Length == data.Length)
                            {
                                int size, startpos, endpos = 0;
                                bool first = true;
                                while (true)
                                {
                                    startpos = strbuf.IndexOf("-----BEGIN CERTIFICATE-----", endpos);
                                    if (startpos != -1)
                                    {
                                        endpos = strbuf.IndexOf("-----END CERTIFICATE-----", startpos);
                                        size = endpos - startpos + "-----END CERTIFICATE-----".Length;
                                    }
                                    else if (first)
                                    {
                                        startpos = 0;
                                        endpos = strbuf.Length;
                                        size = strbuf.Length;
                                    }
                                    else
                                    {
                                        break;
                                    }

                                    byte[] cert = new byte[size];
                                    Buffer.BlockCopy(data, startpos, cert, 0, size);
                                    CaCerts!.Import(cert);
                                    first = false;
                                }
                            }
                            else
                            {
                                CaCerts!.Import(data);
                            }
                        }
                        catch (Exception ex)
                        {
                            throw new InvalidConfigurationException(
                                $"error while attempting to load CA certificate from {certAuthFile}", ex);
                        }
                    }
                }
            }
        }

        internal void TraceStream(SslStream stream, string connInfo)
        {
            var s = new System.Text.StringBuilder();
            s.Append("SSL connection summary");
            if (connInfo.Length > 0)
            {
                s.Append("\n");
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
            _logger.Trace(SecurityTraceCategory, s.ToString());
        }

        internal void VerifyPeer(SslConnectionInfo info, string desc)
        {
            if (_verifyDepthMax > 0 && info.Certs != null && info.Certs.Length > _verifyDepthMax)
            {
                string msg = (info.Incoming ? "incoming" : "outgoing") + " connection rejected:\n" +
                    "length of peer's certificate chain (" + info.Certs.Length + ") exceeds maximum of " +
                    _verifyDepthMax + "\n" + desc;
                if (SecurityTraceLevel >= 1)
                {
                    _logger.Trace(SecurityTraceCategory, msg);
                }
                throw new TransportException(msg);
            }

            if (!_trustManager.Verify(info, desc))
            {
                string msg = (info.Incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                    desc;
                if (SecurityTraceLevel >= 1)
                {
                    _logger.Trace(SecurityTraceCategory, msg);
                }

                throw new TransportException(msg);
            }
        }

        private static SecureString CreateSecureString(string s)
        {
            var result = new SecureString();
            foreach (char ch in s)
            {
                result.AppendChar(ch);
            }
            return result;
        }

        private static X509Certificate2Collection FindCertificates(string prop, StoreLocation storeLocation,
                                                                   string name, string value)
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
                throw new InvalidConfigurationException($"failure while opening X509 store specified by `{prop}'", ex);
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
            var result = new X509Certificate2Collection();
            result.AddRange(store.Certificates);
            try
            {
                if (value != "*")
                {
                    if (value.IndexOf(':') == -1)
                    {
                        throw new FormatException($"no key in `{value}'");
                    }
                    int start = 0;
                    int pos;
                    while ((pos = value.IndexOf(':', start)) != -1)
                    {
                        // Parse the X509FindType.
                        string field = value[start..pos].Trim().ToUpperInvariant();
                        X509FindType findType;
                        if (field.Equals("SUBJECT"))
                        {
                            findType = X509FindType.FindBySubjectName;
                        }
                        else if (field.Equals("SUBJECTDN"))
                        {
                            findType = X509FindType.FindBySubjectDistinguishedName;
                        }
                        else if (field.Equals("ISSUER"))
                        {
                            findType = X509FindType.FindByIssuerName;
                        }
                        else if (field.Equals("ISSUERDN"))
                        {
                            findType = X509FindType.FindByIssuerDistinguishedName;
                        }
                        else if (field.Equals("THUMBPRINT"))
                        {
                            findType = X509FindType.FindByThumbprint;
                        }
                        else if (field.Equals("SUBJECTKEYID"))
                        {
                            findType = X509FindType.FindBySubjectKeyIdentifier;
                        }
                        else if (field.Equals("SERIAL"))
                        {
                            findType = X509FindType.FindBySerialNumber;
                        }
                        else
                        {
                            throw new FormatException($"unknown key in `{value}'");
                        }

                        // Parse the argument.
                        start = pos + 1;
                        while (start < value.Length && (value[start] == ' ' || value[start] == '\t'))
                        {
                            ++start;
                        }
                        if (start == value.Length)
                        {
                            throw new FormatException($"missing argument in `{value}'");
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
                                throw new FormatException("unmatched quote in `{value}'");
                            }
                            ++start;
                            arg = value[start..end];
                            start = end + 1;
                        }
                        else
                        {
                            char[] ws = new char[] { ' ', '\t' };
                            int end = value.IndexOfAny(ws, start);
                            if (end == -1)
                            {
                                arg = value.Substring(start);
                                start = value.Length;
                            }
                            else
                            {
                                arg = value[start..end];
                                start = end + 1;
                            }
                        }

                        // Execute the query.
                        bool validOnly = false;
                        if (findType == X509FindType.FindBySubjectDistinguishedName ||
                            findType == X509FindType.FindByIssuerDistinguishedName)
                        {
                            X500DistinguishedNameFlags[] flags =
                                {
                                    X500DistinguishedNameFlags.None,
                                    X500DistinguishedNameFlags.Reversed,
                                };
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

        private static bool IsAbsolutePath(string path)
        {
            // Skip whitespace
            path = path.Trim();

            if (AssemblyUtil.IsWindows)
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

        private static SslProtocols ParseProtocols(string[]? arr)
        {
            SslProtocols result = SslProtocols.None;

            if (arr != null && arr.Length > 0)
            {
                result = 0;
                for (int i = 0; i < arr.Length; ++i)
                {
                    string protocol;
                    string s = arr[i].ToUpperInvariant();
                    switch (s)
                    {
                        case "TLS1_2":
                        case "TLSV1_2":
                        {
                            protocol = "Tls12";
                            break;
                        }
                        case "TLS1_3":
                        case "TLSV1_3":
                        {
                            protocol = "Tls13";
                            break;
                        }
                        default:
                        {
                            throw new FormatException($"unrecognized SSL protocol `{s}'");
                        }
                    }

                    try
                    {
                        var value = (SslProtocols)Enum.Parse(typeof(SslProtocols), protocol);
                        result |= value;
                    }
                    catch (Exception ex)
                    {
                        throw new FormatException($"unrecognized SSL protocol `{s}'", ex);
                    }
                }
            }
            return result;
        }

        // Parse a string of the form "location.name" into two parts.
        private static void ParseStore(string prop, string store, ref StoreLocation loc, ref StoreName name,
                                       ref string? sname)
        {
            int pos = store.IndexOf('.');
            if (pos == -1)
            {
                throw new InvalidConfigurationException($"property `{prop}' has invalid format");
            }

            string sloc = store.Substring(0, pos).ToUpperInvariant();
            if (sloc.Equals("CURRENTUSER"))
            {
                loc = StoreLocation.CurrentUser;
            }
            else if (sloc.Equals("LOCALMACHINE"))
            {
                loc = StoreLocation.LocalMachine;
            }
            else
            {
                throw new InvalidConfigurationException($"unknown X509 store location `{sloc}' in `{prop}'");
            }

            sname = store.Substring(pos + 1);
            if (sname.Length == 0)
            {
                throw new InvalidConfigurationException($"invalid X509 store name in `{prop}'");
            }

            // Try to convert the name into the StoreName enumeration.
            try
            {
                name = (StoreName)Enum.Parse(typeof(StoreName), sname, true);
                sname = null;
            }
            catch (ArgumentException)
            {
                // Ignore - assume the user is selecting a non-standard store.
            }
        }

        private bool CheckPath(ref string path)
        {
            if (File.Exists(path))
            {
                return true;
            }

            if (_defaultDir.Length > 0 && !IsAbsolutePath(path))
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
    }
}
