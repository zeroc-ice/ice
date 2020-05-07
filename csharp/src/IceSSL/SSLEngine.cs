//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;

using System;
using System.Collections.Generic;
using System.IO;
using System.Security;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    internal class SSLEngine
    {
        internal SSLEngine(IceInternal.ITransportPluginFacade facade)
        {
            _communicator = facade.Communicator;
            _logger = _communicator.Logger;
            _facade = facade;
            _securityTraceLevel = _communicator.GetPropertyAsInt("IceSSL.Trace.Security") ?? 0;
            _securityTraceCategory = "Security";
            _initialized = false;
            _trustManager = new TrustManager(_communicator);
        }

        internal void Initialize()
        {
            if (_initialized)
            {
                return;
            }

            Ice.Communicator ic = Communicator();
            //
            // Check for a default directory. We look in this directory for
            // files mentioned in the configuration.
            //
            _defaultDir = ic.GetProperty("IceSSL.DefaultDir") ?? "";

            string certStoreLocation = ic.GetProperty("IceSSL.CertStoreLocation") ?? "CurrentUser";
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
            _useMachineContext = certStoreLocation == "LocalMachine";

            //
            // Protocols selects which protocols to enable
            //
            string[]? protocols = ic.GetPropertyAsList("IceSSL.Protocols");
            if (protocols != null)
            {
                _protocols = ParseProtocols(protocols);
            }

            //
            // CheckCertName determines whether we compare the name in a peer's
            // certificate against its hostname.
            //
            _checkCertName = ic.GetPropertyAsBool("IceSSL.CheckCertName") ?? false;

            //
            // VerifyDepthMax establishes the maximum length of a peer's certificate
            // chain, including the peer's certificate. A value of 0 means there is
            // no maximum.
            //
            _verifyDepthMax = ic.GetPropertyAsInt("IceSSL.VerifyDepthMax") ?? 3;

            //
            // CheckCRL determines whether the certificate revocation list is checked, and how strictly.
            //
            _checkCRL = ic.GetPropertyAsInt("IceSSL.CheckCRL") ?? 0;

            //
            // Check for a certificate verifier.
            //
            string? certVerifierClass = ic.GetProperty("IceSSL.CertVerifier");
            if (certVerifierClass != null)
            {
                if (_verifier != null)
                {
                    throw new InvalidOperationException("IceSSL: certificate verifier already installed");
                }

                Type? cls = _facade.FindType(certVerifierClass);
                if (cls == null)
                {
                    throw new InvalidConfigurationException(
                        $"IceSSL: unable to load certificate verifier class `{certVerifierClass}'");
                }

                try
                {
                    _verifier = (ICertificateVerifier?)Activator.CreateInstance(cls);
                }
                catch (Exception ex)
                {
                    throw new LoadException(
                        $"IceSSL: unable to instantiate certificate verifier class `{certVerifierClass}", ex);
                }
            }

            //
            // Check for a password callback.
            //
            string? passwordCallbackClass = ic.GetProperty("IceSSL.PasswordCallback");
            if (passwordCallbackClass != null)
            {
                if (_passwordCallback != null)
                {
                    throw new InvalidOperationException("IceSSL: password callback already installed");
                }

                Type? cls = _facade.FindType(passwordCallbackClass);
                if (cls == null)
                {
                    throw new InvalidConfigurationException(
                        $"IceSSL: unable to load password callback class `{passwordCallbackClass}'");
                }

                try
                {
                    _passwordCallback = (IPasswordCallback?)Activator.CreateInstance(cls);
                }
                catch (Exception ex)
                {
                    throw new LoadException(
                        $"IceSSL: unable to load password callback class {passwordCallbackClass}", ex);
                }
            }

            //
            // If the user hasn't supplied a certificate collection, we need to examine
            // the property settings.
            //
            if (_certs == null)
            {
                //
                // If IceSSL.CertFile is defined, load a certificate from a file and
                // add it to the collection.
                //
                // TODO: tracing?
                _certs = new X509Certificate2Collection();
                string? certFile = ic.GetProperty("IceSSL.CertFile");
                string? passwordStr = ic.GetProperty("IceSSL.Password");
                string? findCert = ic.GetProperty("IceSSL.FindCert");
                const string findPrefix = "IceSSL.FindCert.";
                Dictionary<string, string> findCertProps = ic.GetProperties(forPrefix: findPrefix);

                if (certFile != null)
                {
                    if (!CheckPath(ref certFile))
                    {
                        throw new FileNotFoundException($"IceSSL: certificate file not found: `{certFile}'", certFile);
                    }

                    SecureString? password = null;
                    if (passwordStr != null)
                    {
                        password = CreateSecureString(passwordStr);
                    }
                    else if (_passwordCallback != null)
                    {
                        password = _passwordCallback.GetPassword(certFile);
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

                        if (password != null)
                        {
                            cert = new X509Certificate2(certFile, password, importFlags);
                        }
                        else
                        {
                            cert = new X509Certificate2(certFile, "", importFlags);
                        }
                        _certs.Add(cert);
                    }
                    catch (CryptographicException ex)
                    {
                        throw new InvalidConfigurationException(
                            $"IceSSL: error while attempting to load certificate from `{certFile}'", ex);
                    }
                }
                else if (findCert != null)
                {
                    string certStore = ic.GetProperty("IceSSL.CertStore") ?? "My";
                    _certs.AddRange(FindCertificates("IceSSL.FindCert", storeLocation, certStore, findCert));
                    if (_certs.Count == 0)
                    {
                        throw new InvalidConfigurationException("IceSSL: no certificates found");
                    }
                }
                else if (findCertProps.Count > 0)
                {
                    //
                    // If IceSSL.FindCert.* properties are defined, add the selected certificates
                    // to the collection.
                    //
                    foreach (KeyValuePair<string, string> entry in findCertProps)
                    {
                        string name = entry.Key;
                        string val = entry.Value;
                        if (val.Length > 0)
                        {
                            string storeSpec = name.Substring(findPrefix.Length);
                            StoreLocation storeLoc = 0;
                            StoreName storeName = 0;
                            string? sname = null;
                            ParseStore(name, storeSpec, ref storeLoc, ref storeName, ref sname);
                            if (sname == null)
                            {
                                sname = storeName.ToString();
                            }
                            X509Certificate2Collection coll = FindCertificates(name, storeLoc, sname, val);
                            _certs.AddRange(coll);
                        }
                    }
                    if (_certs.Count == 0)
                    {
                        throw new InvalidConfigurationException("IceSSL: no certificates found");
                    }
                }
            }

            if (_caCerts == null)
            {
                string? certAuthFile = ic.GetProperty("IceSSL.CAs");
                if (certAuthFile == null)
                {
                    certAuthFile = ic.GetProperty("IceSSL.CertAuthFile");
                }

                if (certAuthFile != null || !(ic.GetPropertyAsBool("IceSSL.UsePlatformCAs") ?? false))
                {
                    _caCerts = new X509Certificate2Collection();
                }

                if (certAuthFile != null)
                {
                    if (!CheckPath(ref certAuthFile))
                    {
                        throw new FileNotFoundException("IceSSL: CA certificate file not found: `{certAuthFile}'",
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
                                _caCerts!.Import(cert);
                                first = false;
                            }
                        }
                        else
                        {
                            _caCerts!.Import(data);
                        }
                    }
                    catch (Exception ex)
                    {
                        throw new InvalidConfigurationException(
                            $"IceSSL: error while attempting to load CA certificate from {certAuthFile}", ex);
                    }
                }
            }
            _initialized = true;
        }

        internal bool UseMachineContext() => _useMachineContext;

        internal X509Certificate2Collection? CaCerts() => _caCerts;

        internal void SetCACertificates(X509Certificate2Collection caCerts)
        {
            if (_initialized)
            {
                throw new InvalidOperationException("IceSSL: plug-in is already initialized");
            }

            _caCerts = caCerts;
        }

        internal void SetCertificates(X509Certificate2Collection certs)
        {
            if (_initialized)
            {
                throw new InvalidOperationException("IceSSL: plug-in is already initialized");
            }

            _certs = certs;
        }

        internal void SetCertificateVerifier(ICertificateVerifier verifier) => _verifier = verifier;

        internal ICertificateVerifier? GetCertificateVerifier() => _verifier;

        internal bool GetCheckCertName() => _checkCertName;

        internal void SetPasswordCallback(IPasswordCallback callback) => _passwordCallback = callback;

        internal IPasswordCallback? GetPasswordCallback() => _passwordCallback;

        internal Ice.Communicator Communicator() => _facade.Communicator;

        internal int SecurityTraceLevel() => _securityTraceLevel;

        internal string SecurityTraceCategory() => _securityTraceCategory;

        internal bool Initialized() => _initialized;

        internal X509Certificate2Collection? Certs() => _certs;

        internal SslProtocols Protocols() => _protocols;

        internal int CheckCRL() => _checkCRL;

        internal void TraceStream(System.Net.Security.SslStream stream, string connInfo)
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
            _logger.Trace(_securityTraceCategory, s.ToString());
        }

        internal void VerifyPeer(ConnectionInfo info, string desc)
        {
            if (_verifyDepthMax > 0 && info.Certs != null && info.Certs.Length > _verifyDepthMax)
            {
                string msg = (info.Incoming ? "incoming" : "outgoing") + " connection rejected:\n" +
                    "length of peer's certificate chain (" + info.Certs.Length + ") exceeds maximum of " +
                    _verifyDepthMax + "\n" + desc;
                if (_securityTraceLevel >= 1)
                {
                    _logger.Trace(_securityTraceCategory, msg);
                }
                throw new Ice.SecurityException(msg);
            }

            if (!_trustManager.Verify(info, desc))
            {
                string msg = (info.Incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                    desc;
                if (_securityTraceLevel >= 1)
                {
                    _logger.Trace(_securityTraceCategory, msg);
                }

                throw new Ice.SecurityException($"IceSSL: {msg}");
            }

            if (_verifier != null && !_verifier.Verify(info))
            {
                string msg = (info.Incoming ? "incoming" : "outgoing") +
                    " connection rejected by certificate verifier\n" + desc;
                if (_securityTraceLevel >= 1)
                {
                    _logger.Trace(_securityTraceCategory, msg);
                }

                throw new Ice.SecurityException($"IceSSL: {msg}");
            }
        }

        //
        // Parse a string of the form "location.name" into two parts.
        //
        private static void ParseStore(string prop, string store, ref StoreLocation loc, ref StoreName name,
                                       ref string? sname)
        {
            int pos = store.IndexOf('.');
            if (pos == -1)
            {
                throw new InvalidConfigurationException($"IceSSL: property `{prop}' has invalid format");
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
                throw new InvalidConfigurationException($"IceSSL: unknown store location `{sloc}' in `{prop}'");
            }

            sname = store.Substring(pos + 1);
            if (sname.Length == 0)
            {
                throw new InvalidConfigurationException($"IceSSL: invalid store name in `{prop}'");
            }

            //
            // Try to convert the name into the StoreName enumeration.
            //
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

        private static bool IsAbsolutePath(string path)
        {
            //
            // Skip whitespace
            //
            path = path.Trim();

            if (AssemblyUtil.IsWindows)
            {
                //
                // We need at least 3 non-whitespace characters to have an absolute path
                //
                if (path.Length < 3)
                {
                    return false;
                }

                //
                // Check for X:\ path ('\' may have been converted to '/')
                //
                if ((path[0] >= 'A' && path[0] <= 'Z') || (path[0] >= 'a' && path[0] <= 'z'))
                {
                    return path[1] == ':' && (path[2] == '\\' || path[2] == '/');
                }
            }

            //
            // Check for UNC path
            //
            return (path[0] == '\\' && path[1] == '\\') || path[0] == '/';
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

        private SslProtocols ParseProtocols(string[] arr)
        {
            SslProtocols result = SslProtocols.None;

            if (arr.Length > 0)
            {
                result = 0;
                for (int i = 0; i < arr.Length; ++i)
                {
                    string protocol;
                    string s = arr[i].ToUpperInvariant();
                    switch (s)
                    {
                        case "SSL3":
                        case "SSLV3":
                            {
                                protocol = "Ssl3";
                                break;
                            }
                        case "TLS":
                        case "TLS1":
                        case "TLS1_0":
                        case "TLSV1":
                        case "TLSV1_0":
                            {
                                protocol = "Tls";
                                break;
                            }
                        case "TLS1_1":
                        case "TLSV1_1":
                            {
                                protocol = "Tls11";
                                break;
                            }
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
                                throw new FormatException($"IceSSL: unrecognized protocol `{s}'");
                            }
                    }

                    try
                    {
                        var value = (SslProtocols)Enum.Parse(typeof(SslProtocols), protocol);
                        result |= value;
                    }
                    catch (Exception ex)
                    {
                        throw new FormatException($"IceSSL: unrecognized protocol `{s}'", ex);
                    }
                }
            }
            return result;
        }

        private static X509Certificate2Collection FindCertificates(string prop, StoreLocation storeLocation,
                                                                   string name, string value)
        {
            //
            // Open the X509 certificate store.
            //
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
                throw new InvalidConfigurationException($"IceSSL: failure while opening store specified by `{prop}'",
                    ex);
            }

            //
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
            //
            var result = new X509Certificate2Collection();
            result.AddRange(store.Certificates);
            try
            {
                if (value != "*")
                {
                    if (value.IndexOf(':') == -1)
                    {
                        throw new FormatException($"IceSSL: no key in `{value}'");
                    }
                    int start = 0;
                    int pos;
                    while ((pos = value.IndexOf(':', start)) != -1)
                    {
                        //
                        // Parse the X509FindType.
                        //
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
                            throw new FormatException($"IceSSL: unknown key in `{value}'");
                        }

                        //
                        // Parse the argument.
                        //
                        start = pos + 1;
                        while (start < value.Length && (value[start] == ' ' || value[start] == '\t'))
                        {
                            ++start;
                        }
                        if (start == value.Length)
                        {
                            throw new FormatException($"IceSSL: missing argument in `{value}'");
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
                                throw new FormatException("IceSSL: unmatched quote in `{value}'");
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

                        //
                        // Execute the query.
                        //
                        // TODO: allow user to specify a value for validOnly?
                        //
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

        private static SecureString CreateSecureString(string s)
        {
            var result = new SecureString();
            foreach (char ch in s)
            {
                result.AppendChar(ch);
            }
            return result;
        }

        private readonly Ice.Communicator _communicator;
        private readonly Ice.ILogger _logger;
        private readonly IceInternal.ITransportPluginFacade _facade;
        private readonly int _securityTraceLevel;
        private readonly string _securityTraceCategory;
        private bool _initialized;
        private string _defaultDir = string.Empty;
        private SslProtocols _protocols;
        private bool _checkCertName;
        private int _verifyDepthMax;
        private int _checkCRL;
        private X509Certificate2Collection? _certs;
        private bool _useMachineContext;
        private X509Certificate2Collection? _caCerts;
        private ICertificateVerifier? _verifier;
        private IPasswordCallback? _passwordCallback;
        private readonly TrustManager _trustManager;
    }
}
