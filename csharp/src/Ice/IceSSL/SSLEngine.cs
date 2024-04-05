// Copyright (c) ZeroC, Inc.

using System.Diagnostics;
using System.Security;
using System.Security.Authentication;
using System.Security.Cryptography;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL;

internal class SSLEngine
{
    internal SSLEngine(Ice.Communicator communicator)
    {
        _communicator = communicator;
        _logger = communicator.getLogger();
        _securityTraceLevel = _communicator.getProperties().getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
        _securityTraceCategory = "Security";
        _initialized = false;
        _trustManager = new TrustManager(_communicator);
    }

    internal void initialize()
    {
        if (_initialized)
        {
            return;
        }

        const string prefix = "IceSSL.";
        Ice.Properties properties = communicator().getProperties();

        //
        // Check for a default directory. We look in this directory for
        // files mentioned in the configuration.
        //
        _defaultDir = properties.getProperty(prefix + "DefaultDir");

        string certStoreLocation = properties.getPropertyWithDefault(prefix + "CertStoreLocation", "CurrentUser");
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

        //
        // CheckCertName determines whether we compare the name in a peer's
        // certificate against its hostname.
        //
        _checkCertName = properties.getPropertyAsIntWithDefault(prefix + "CheckCertName", 0) > 0;

        //
        // VerifyDepthMax establishes the maximum length of a peer's certificate
        // chain, including the peer's certificate. A value of 0 means there is
        // no maximum.
        //
        _verifyDepthMax = properties.getPropertyAsIntWithDefault(prefix + "VerifyDepthMax", 3);

        //
        // CheckCRL determines whether the certificate revocation list is checked, and how strictly.
        //
        _checkCRL = properties.getPropertyAsIntWithDefault(prefix + "CheckCRL", 0);

        Debug.Assert(_certs == null);
        // If IceSSL.CertFile is defined, load a certificate from a file and add it to the collection.
        _certs = [];
        string certFile = properties.getProperty(prefix + "CertFile");
        string passwordStr = properties.getProperty(prefix + "Password");
        string findCert = properties.getProperty(prefix + "FindCert");

        if (certFile.Length > 0)
        {
            if (!checkPath(ref certFile))
            {
                throw new Ice.InitializationException($"IceSSL: certificate file not found: {certFile}");
            }

            SecureString password = null;
            if (passwordStr.Length > 0)
            {
                password = createSecureString(passwordStr);
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
            string certStore = properties.getPropertyWithDefault("IceSSL.CertStore", "My");
            _certs.AddRange(findCertificates("IceSSL.FindCert", storeLocation, certStore, findCert));
            if (_certs.Count == 0)
            {
                throw new Ice.InitializationException("IceSSL: no certificates found");
            }
        }

        Debug.Assert(_caCerts == null);
        string certAuthFile = properties.getProperty(prefix + "CAs");
        if (certAuthFile.Length > 0 || properties.getPropertyAsInt(prefix + "UsePlatformCAs") <= 0)
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
                using (FileStream fs = File.OpenRead(certAuthFile))
                {
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
                            _caCerts.Import(cert);
                            first = false;
                        }
                    }
                    else
                    {
                        _caCerts.Import(data);
                    }
                }
            }
            catch (Exception ex)
            {
                throw new Ice.InitializationException(
                    $"IceSSL: error while attempting to load CA certificate from {certAuthFile}",
                    ex);
            }
        }
        _initialized = true;
    }

    internal bool useMachineContext()
    {
        return _useMachineContext;
    }

    internal X509Certificate2Collection caCerts()
    {
        return _caCerts;
    }

    internal bool getCheckCertName()
    {
        return _checkCertName;
    }

    internal Ice.Communicator communicator()
    {
        return _communicator;
    }

    internal int securityTraceLevel()
    {
        return _securityTraceLevel;
    }

    internal string securityTraceCategory()
    {
        return _securityTraceCategory;
    }

    internal bool initialized()
    {
        return _initialized;
    }

    internal X509Certificate2Collection certs()
    {
        return _certs;
    }

    internal int checkCRL()
    {
        return _checkCRL;
    }

    internal void traceStream(System.Net.Security.SslStream stream, string connInfo)
    {
        System.Text.StringBuilder s = new System.Text.StringBuilder();
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

    internal void verifyPeer(string address, IceSSL.ConnectionInfo info, string desc)
    {
        if (_verifyDepthMax > 0 && info.certs != null && info.certs.Length > _verifyDepthMax)
        {
            string msg = (info.incoming ? "incoming" : "outgoing") + " connection rejected:\n" +
                "length of peer's certificate chain (" + info.certs.Length + ") exceeds maximum of " +
                _verifyDepthMax + "\n" + desc;
            if (_securityTraceLevel >= 1)
            {
                _logger.trace(_securityTraceCategory, msg);
            }
            throw new Ice.SecurityException(msg);
        }

        if (!_trustManager.verify(info, desc))
        {
            string msg = (info.incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                desc;
            if (_securityTraceLevel >= 1)
            {
                _logger.trace(_securityTraceCategory, msg);
            }

            throw new Ice.SecurityException($"IceSSL: {msg}");
        }
    }

    private static bool isAbsolutePath(string path)
    {
        //
        // Skip whitespace
        //
        path = path.Trim();

        if (IceInternal.AssemblyUtil.isWindows)
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

    private static X509Certificate2Collection findCertificates(string prop, StoreLocation storeLocation,
                                                               string name, string value)
    {
        //
        // Open the X509 certificate store.
        //
        X509Store store = null;
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
        X509Certificate2Collection result = [.. store.Certificates];
        try
        {
            if (value != "*")
            {
                if (!value.Contains(':'))
                {
                    throw new Ice.InitializationException($"IceSSL: no key in `{value}'");
                }
                int start = 0;
                int pos;
                while ((pos = value.IndexOf(':', start)) != -1)
                {
                    //
                    // Parse the X509FindType.
                    //
                    string field = value.Substring(start, pos - start).Trim().ToUpperInvariant();
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
                        arg = value.Substring(start, end - start);
                        start = end + 1;
                    }
                    else
                    {
                        char[] ws = [' ', '\t'];
                        int end = value.IndexOfAny(ws, start);
                        if (end == -1)
                        {
                            arg = value.Substring(start);
                            start = value.Length;
                        }
                        else
                        {
                            arg = value.Substring(start, end - start);
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
                        X500DistinguishedNameFlags[] flags = {
                            X500DistinguishedNameFlags.None,
                            X500DistinguishedNameFlags.Reversed,
                        };
                        X500DistinguishedName dn = new X500DistinguishedName(arg);
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

    private readonly Ice.Communicator _communicator;
    private readonly Ice.Logger _logger;
    private readonly int _securityTraceLevel;
    private readonly string _securityTraceCategory;
    private bool _initialized;
    private string _defaultDir;
    private bool _checkCertName;
    private int _verifyDepthMax;
    private int _checkCRL;
    private X509Certificate2Collection _certs;
    private bool _useMachineContext;
    private X509Certificate2Collection _caCerts;
    private readonly TrustManager _trustManager;
}
