// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Collections;
    using System.Collections.Generic;
    using System.IO;
    using System.Security;
    using System.Security.Authentication;
    using System.Security.Cryptography;
    using System.Security.Cryptography.X509Certificates;

    internal class Instance
    {
        internal Instance(Ice.Communicator communicator)
        {
            _logger = communicator.getLogger();
            _facade = Ice.Util.getProtocolPluginFacade(communicator);
            _securityTraceLevel = communicator.getProperties().getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
            _securityTraceCategory = "Security";
            _initialized = false;
            _trustManager = new TrustManager(communicator);

            //
            // Register the endpoint factory. We have to do this now, rather than
            // in initialize, because the communicator may need to interpret
            // proxies before the plugin is fully initialized.
            //
            _facade.addEndpointFactory(new EndpointFactoryI(this));
        }

        internal void initialize()
        {
            if(_initialized)
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

            //
            // Process IceSSL.ImportCert.* properties.
            //
            Dictionary<string, string> certs = properties.getPropertiesForPrefix(prefix + "ImportCert.");
            foreach(KeyValuePair<string, string> entry in certs)
            {
                string name = entry.Key;
                string val = entry.Value;
                if(val.Length > 0)
                {
                    importCertificate(name, val);
                }
            }

            //
            // Select protocols.
            //
            _protocols = parseProtocols(prefix + "Protocols");

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
            _verifyDepthMax = properties.getPropertyAsIntWithDefault(prefix + "VerifyDepthMax", 2);

            //
            // CheckCRL determines whether the certificate revocation list is checked, and how strictly.
            //
            _checkCRL = properties.getPropertyAsIntWithDefault(prefix + "CheckCRL", 0);

            //
            // Check for a certificate verifier.
            //
            string certVerifierClass = properties.getProperty(prefix + "CertVerifier");
            if(certVerifierClass.Length > 0)
            {
                if(_verifier != null)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: certificate verifier already installed";
                    throw e;
                }

                Type cls = IceInternal.AssemblyUtil.findType(certVerifierClass);
                if(cls == null)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: unable to load certificate verifier class " + certVerifierClass;
                    throw e;
                }

                try
                {
                    _verifier = (CertificateVerifier)IceInternal.AssemblyUtil.createInstance(cls);
                }
                catch(Exception ex)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
                    e.reason = "IceSSL: unable to instantiate certificate verifier class " + certVerifierClass;
                    throw e;
                }

                if(_verifier == null)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: unable to instantiate certificate verifier class " + certVerifierClass;
                    throw e;
                }
            }

            //
            // Check for a password callback.
            //
            string passwordCallbackClass = properties.getProperty(prefix + "PasswordCallback");
            if(passwordCallbackClass.Length > 0)
            {
                if(_passwordCallback != null)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: password callback already installed";
                    throw e;
                }

                Type cls = IceInternal.AssemblyUtil.findType(passwordCallbackClass);
                if(cls == null)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: unable to load password callback class " + passwordCallbackClass;
                    throw e;
                }

                try
                {
                    _passwordCallback = (PasswordCallback)IceInternal.AssemblyUtil.createInstance(cls);
                }
                catch(Exception ex)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
                    e.reason = "IceSSL: unable to load password callback class " + passwordCallbackClass;
                    throw e;
                }

                if(_passwordCallback == null)
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: unable to load password callback class " + passwordCallbackClass;
                    throw e;
                }
            }

            //
            // If the user hasn't supplied a certificate collection, we need to examine
            // the property settings.
            //
            if(_certs == null)
            {
                //
                // If IceSSL.CertFile is defined, load a certificate from a file and
                // add it to the collection.
                //
                // TODO: tracing?
                _certs = new X509Certificate2Collection();
                string certFile = properties.getProperty(prefix + "CertFile");
                string passwordStr = properties.getProperty(prefix + "Password");
                if(certFile.Length > 0)
                {
                    if(!checkPath(ref certFile))
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: certificate file not found: " + certFile;
                        throw e;
                    }

                    SecureString password = null;
                    if(passwordStr.Length > 0)
                    {
                        password = createSecureString(passwordStr);
                    }
                    else if(_passwordCallback != null)
                    {
                        password = _passwordCallback.getPassword(certFile);
                    }

                    try
                    {
                        X509Certificate2 cert;
                        if(password != null)
                        {
                            cert = new X509Certificate2(certFile, password);
                        }
                        else
                        {
                            cert = new X509Certificate2(certFile);
                        }
                        _certs.Add(cert);
                    }
                    catch(CryptographicException ex)
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
                        e.reason = "IceSSL: error while attempting to load certificate from " + certFile;
                        throw e;
                    }
                }

                //
                // If IceSSL.FindCert.* properties are defined, add the selected certificates
                // to the collection.
                //
                // TODO: tracing?
                const string findPrefix = prefix + "FindCert.";
                Dictionary<string, string> certProps = properties.getPropertiesForPrefix(findPrefix);
                if(certProps.Count > 0)
                {
                    foreach(KeyValuePair<string, string> entry in certProps)
                    {
                        string name = entry.Key;
                        string val = entry.Value;
                        if(val.Length > 0)
                        {
                            string storeSpec = name.Substring(findPrefix.Length);
                            X509Certificate2Collection coll = findCertificates(name, storeSpec, val);
                            _certs.AddRange(coll);
                        }
                    }
                    if(_certs.Count == 0)
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: no certificates found";
                        throw e;
                    }
                }
            }

            _initialized = true;
        }

        internal void setCertificates(X509Certificate2Collection certs)
        {
            if(_initialized)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: plugin is already initialized";
                throw e;
            }

            _certs = certs;
        }

        internal void setCertificateVerifier(CertificateVerifier verifier)
        {
            _verifier = verifier;
        }

        internal CertificateVerifier getCertificateVerifier()
        {
            return _verifier;
        }

        internal void setPasswordCallback(PasswordCallback callback)
        {
            _passwordCallback = callback;
        }

        internal PasswordCallback getPasswordCallback()
        {
            return _passwordCallback;
        }

        internal Ice.Communicator communicator()
        {
            return _facade.getCommunicator();
        }

        internal IceInternal.EndpointHostResolver endpointHostResolver()
        {
            return _facade.getEndpointHostResolver();
        }

        internal int protocolSupport()
        {
            return _facade.getProtocolSupport();
        }

        internal string defaultHost()
        {
            return _facade.getDefaultHost();
        }

        internal int networkTraceLevel()
        {
            return _facade.getNetworkTraceLevel();
        }

        internal string networkTraceCategory()
        {
            return _facade.getNetworkTraceCategory();
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

        internal SslProtocols protocols()
        {
            return _protocols;
        }

        internal int checkCRL()
        {
            return _checkCRL;
        }

        internal bool checkCertName()
        {
            return _checkCertName;
        }

        internal void traceStream(System.Net.Security.SslStream stream, string connInfo)
        {
            System.Text.StringBuilder s = new System.Text.StringBuilder();
            s.Append("SSL connection summary");
            if(connInfo.Length > 0)
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
            communicator().getLogger().trace(_securityTraceCategory, s.ToString());
        }

        internal void verifyPeer(ConnectionInfo info, System.Net.Sockets.Socket fd, bool incoming)
        {
            if(_verifyDepthMax > 0 && info.certs != null && info.certs.Length > _verifyDepthMax)
            {
                string msg = (incoming ? "incoming" : "outgoing") + " connection rejected:\n" +
                    "length of peer's certificate chain (" + info.certs.Length + ") exceeds maximum of " +
                    _verifyDepthMax + "\n" +
                    IceInternal.Network.fdToString(fd);
                if(_securityTraceLevel >= 1)
                {
                    _logger.trace(_securityTraceCategory, msg);
                }
                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = msg;
                throw ex;
            }

            if(!_trustManager.verify(info))
            {
                string msg = (incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                    IceInternal.Network.fdToString(fd);
                if(_securityTraceLevel >= 1)
                {
                    _logger.trace(_securityTraceCategory, msg);
                }

                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: " + msg;
                throw ex;
            }

            if(_verifier != null && !_verifier.verify(info))
            {
                string msg = (incoming ? "incoming" : "outgoing") + " connection rejected by certificate verifier\n" +
                    IceInternal.Network.fdToString(fd);
                if(_securityTraceLevel >= 1)
                {
                    _logger.trace(_securityTraceCategory, msg);
                }

                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: " + msg;
                throw ex;
            }
        }

        //
        // Parse a string of the form "location.name" into two parts.
        //
        internal void parseStore(string prop, string store, ref StoreLocation loc, ref StoreName name, ref string sname)
        {
            int pos = store.IndexOf('.');
            if(pos == -1)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: property `" + prop + "' has invalid format";
                throw e;
            }

            string sloc = store.Substring(0, pos).ToLower();
            if(sloc.Equals("currentuser"))
            {
                loc = StoreLocation.CurrentUser;
            }
            else if(sloc.Equals("localmachine"))
            {
                loc = StoreLocation.LocalMachine;
            }
            else
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: unknown store location `" + sloc + "' in " + prop;
                throw e;
            }

            sname = store.Substring(pos + 1);
            if(sname.Length == 0)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: invalid store name in " + prop;
                throw e;
            }

            //
            // Try to convert the name into the StoreName enumeration.
            //
            try
            {
                name = (StoreName)Enum.Parse(typeof(StoreName), sname, true);
                sname = null;
            }
            catch(ArgumentException)
            {
                // Ignore - assume the user is selecting a non-standard store.
            }
        }

        private bool checkPath(ref string path)
        {
            if(File.Exists(path))
            {
                return true;
            }

            if(_defaultDir.Length > 0)
            {
                string s = _defaultDir + Path.DirectorySeparatorChar + path;
                if(File.Exists(s))
                {
                    path = s;
                    return true;
                }
            }

            return false;
        }

        private void importCertificate(string propName, string propValue)
        {
            //
            // Expecting a property of the following form:
            //
            // IceSSL.ImportCert.<location>.<name>=<file>[;password]
            //
            const string prefix = "IceSSL.ImportCert.";
            StoreLocation loc = 0;
            StoreName name = 0;
            string sname = null;
            parseStore(propName, propName.Substring(prefix.Length), ref loc, ref name, ref sname);

            //
            // Extract the filename and password. Either or both can be quoted.
            //
            string[] arr = splitString(propValue, ';');
            if(arr == null)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: unmatched quote in `" + propValue + "'";
                throw e;
            }
            if(arr.Length == 0)
            {
                return;
            }
            string file = arr[0];
            string passwordStr = null;
            if(arr.Length > 1)
            {
                passwordStr = arr[1];
            }

            //
            // Open the X509 certificate store.
            //
            X509Store store = null;
            try
            {
                if(sname != null)
                {
                    store = new X509Store(sname, loc);
                }
                else
                {
                    store = new X509Store(name, loc);
                }
                store.Open(OpenFlags.ReadWrite);
            }
            catch(Exception ex)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
                e.reason = "IceSSL: failure while opening store specified by " + propName;
                throw e;
            }

            if(!checkPath(ref file))
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: certificate file not found:\n" + file;
                throw e;
            }

            SecureString password = null;
            if(passwordStr != null)
            {
                password = createSecureString(passwordStr);
            }
            else if(_passwordCallback != null)
            {
                password = _passwordCallback.getImportPassword(file);
            }

            //
            // Add the certificate to the store.
            //
            try
            {
                X509Certificate2 cert;
                if(password != null)
                {
                    cert = new X509Certificate2(file, password);
                }
                else
                {
                    cert = new X509Certificate2(file);
                }
                store.Add(cert);
            }
            catch(Exception ex)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
                e.reason = "IceSSL: failure while adding certificate file:\n" + file;
                throw e;
            }
            finally
            {
                store.Close();
            }
        }

        //
        // Split strings using a delimiter. Quotes are supported.
        // Returns null for an unmatched quote.
        //
        private string[] splitString(string str, char delim)
        {
            ArrayList l = new ArrayList();
            char[] arr = new char[str.Length];
            int pos = 0;

            while(pos < str.Length)
            {
                int n = 0;
                char quoteChar = '\0';
                if(str[pos] == '"' || str[pos] == '\'')
                {
                    quoteChar = str[pos];
                    ++pos;
                }
                bool trim = true;
                while(pos < str.Length)
                {
                    if(quoteChar != '\0' && str[pos] == '\\' && pos + 1 < str.Length && str[pos + 1] == quoteChar)
                    {
                        ++pos;
                    }
                    else if(quoteChar != '\0' && str[pos] == quoteChar)
                    {
                        trim = false;
                        ++pos;
                        quoteChar = '\0';
                        break;
                    }
                    else if(str[pos] == delim)
                    {
                        if(quoteChar == '\0')
                        {
                            ++pos;
                            break;
                        }
                    }
                    if(pos < str.Length)
                    {
                        arr[n++] = str[pos++];
                    }
                }
                if(quoteChar != '\0')
                {
                    return null; // Unmatched quote.
                }
                if(n > 0)
                {
                    string s = new string(arr, 0, n);
                    if(trim)
                    {
                        s = s.Trim();
                    }
                    if(s.Length > 0)
                    {
                        l.Add(s);
                    }
                }
            }

            return (string[])l.ToArray(typeof(string));
        }

        private SslProtocols parseProtocols(string property)
        {
            SslProtocols result = SslProtocols.Default;
            string[] arr = communicator().getProperties().getPropertyAsList(property);
            if(arr.Length > 0)
            {
                result = 0;
                for(int i = 0; i < arr.Length; ++i)
                {
                    string s = arr[i].ToLower();
                    if(s.Equals("ssl3") || s.Equals("sslv3"))
                    {
                        result |= SslProtocols.Ssl3;
                    }
                    else if(s.Equals("tls") || s.Equals("tls1") || s.Equals("tlsv1"))
                    {
                        result |= SslProtocols.Tls;
                    }
                    else
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: unrecognized protocol `" + s + "'";
                        throw e;
                    }
                }
            }
            return result;
        }

        private X509Certificate2Collection findCertificates(string prop, string storeSpec, string value)
        {
            StoreLocation storeLoc = 0;
            StoreName storeName = 0;
            string storeNameStr = null;
            parseStore(prop, storeSpec, ref storeLoc, ref storeName, ref storeNameStr);

            //
            // Open the X509 certificate store.
            //
            X509Store store = null;
            try
            {
                if(storeNameStr != null)
                {
                    store = new X509Store(storeNameStr, storeLoc);
                }
                else
                {
                    store = new X509Store(storeName, storeLoc);
                }
                store.Open(OpenFlags.ReadOnly);
            }
            catch(Exception ex)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException(ex);
                e.reason = "IceSSL: failure while opening store specified by " + prop;
                throw e;
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
            X509Certificate2Collection result = new X509Certificate2Collection();
            result.AddRange(store.Certificates);
            try
            {
                if(value != "*")
                {
                    int start = 0;
                    int pos;
                    while((pos = value.IndexOf(':', start)) != -1)
                    {
                        //
                        // Parse the X509FindType.
                        //
                        string field = value.Substring(start, pos - start).Trim().ToLower();
                        X509FindType findType;
                        if(field.Equals("subject"))
                        {
                            findType = X509FindType.FindBySubjectName;
                        }
                        else if(field.Equals("subjectdn"))
                        {
                            findType = X509FindType.FindBySubjectDistinguishedName;
                        }
                        else if(field.Equals("issuer"))
                        {
                            findType = X509FindType.FindByIssuerName;
                        }
                        else if(field.Equals("issuerdn"))
                        {
                            findType = X509FindType.FindByIssuerDistinguishedName;
                        }
                        else if(field.Equals("thumbprint"))
                        {
                            findType = X509FindType.FindByThumbprint;
                        }
                        else if(field.Equals("subjectkeyid"))
                        {
                            findType = X509FindType.FindBySubjectKeyIdentifier;
                        }
                        else if(field.Equals("serial"))
                        {
                            findType = X509FindType.FindBySerialNumber;
                        }
                        else
                        {
                            Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                            e.reason = "IceSSL: unknown key in `" + value + "'";
                            throw e;
                        }

                        //
                        // Parse the argument.
                        //
                        start = pos + 1;
                        while(start < value.Length && (value[start] == ' ' || value[start] == '\t'))
                        {
                            ++start;
                        }
                        if(start == value.Length)
                        {
                            Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                            e.reason = "IceSSL: missing argument in `" + value + "'";
                            throw e;
                        }

                        string arg;
                        if(value[start] == '"' || value[start] == '\'')
                        {
                            int end = start;
                            ++end;
                            while(end < value.Length)
                            {
                                if(value[end] == value[start] && value[end - 1] != '\\')
                                {
                                    break;
                                }
                                ++end;
                            }
                            if(end == value.Length || value[end] != value[start])
                            {
                                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                                e.reason = "IceSSL: unmatched quote in `" + value + "'";
                                throw e;
                            }
                            ++start;
                            arg = value.Substring(start, end - start);
                            start = end + 1;
                        }
                        else
                        {
                            char[] ws = new char[] { ' ', '\t' };
                            int end = value.IndexOfAny(ws, start);
                            if(end == -1)
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
                        result = result.Find(findType, arg, validOnly);
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
            SecureString result = new SecureString();
            foreach(char ch in s)
            {
                result.AppendChar(ch);
            }
            return result;
        }

        private Ice.Logger _logger;
        private IceInternal.ProtocolPluginFacade _facade;
        private int _securityTraceLevel;
        private string _securityTraceCategory;
        private bool _initialized;
        private string _defaultDir;
        private SslProtocols _protocols;
        private bool _checkCertName;
        private int _verifyDepthMax;
        private int _checkCRL;
        private X509Certificate2Collection _certs;
        private CertificateVerifier _verifier;
        private PasswordCallback _passwordCallback;
        private TrustManager _trustManager;
    }
}
