// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System;
    using System.Collections;
    using System.IO;
    using System.Security.Authentication;
    using System.Security.Cryptography;
    using System.Security.Cryptography.X509Certificates;

    internal class Instance
    {
        internal Instance(Ice.Communicator communicator)
        {
            logger_ = communicator.getLogger();
            facade_ = Ice.Util.getProtocolPluginFacade(communicator);
            securityTraceLevel_ = communicator.getProperties().getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
            securityTraceCategory_ = "Security";
            initialized_ = false;
            trustManager_ = new TrustManager(communicator);

            //
            // Register the endpoint factory. We have to do this now, rather than
            // in initialize, because the communicator may need to interpret
            // proxies before the plugin is fully initialized.
            //
            facade_.addEndpointFactory(new EndpointFactoryI(this));
        }

        internal void initialize()
        {
            if(initialized_)
            {
                return;
            }

            const string prefix = "IceSSL.";
            Ice.Properties properties = communicator().getProperties();

            //
            // Check for a default directory. We look in this directory for
            // files mentioned in the configuration.
            //
            defaultDir_ = properties.getProperty(prefix + "DefaultDir");

            //
            // Process IceSSL.ImportCert.* properties.
            //
            Ice.PropertyDict certs = properties.getPropertiesForPrefix(prefix + "ImportCert.");
            foreach(DictionaryEntry entry in certs)
            {
                string name = (string)entry.Key;
                string val = (string)entry.Value;
                if(val.Length > 0)
                {
                    importCertificate(name, val);
                }
            }

            //
            // Select protocols.
            //
            protocols_ = parseProtocols(prefix + "Protocols");

            //
            // CheckCertName determines whether we compare the name in a peer's
            // certificate against its hostname.
            //
            checkCertName_ = properties.getPropertyAsIntWithDefault(prefix + "CheckCertName", 0) > 0;

            //
            // VerifyDepthMax establishes the maximum length of a peer's certificate
            // chain, including the peer's certificate. A value of 0 means there is
            // no maximum.
            //
            verifyDepthMax_ = properties.getPropertyAsIntWithDefault(prefix + "VerifyDepthMax", 2);

            //
            // CheckCRL determines whether the certificate revocation list is checked.
            //
            checkCRL_ = properties.getPropertyAsIntWithDefault(prefix + "CheckCRL", 0) > 0;

            //
            // If the user hasn't supplied a certificate collection, we need to examine
            // the property settings.
            //
            if(certs_ == null)
            {
                //
                // If IceSSL.CertFile is defined, load a certificate from a file and
                // add it to the collection.
                //
                // TODO: tracing?
                certs_ = new X509Certificate2Collection();
                string certFile = properties.getProperty(prefix + "CertFile");
                string password = properties.getProperty(prefix + "Password");
                if(certFile.Length > 0)
                {
                    if(!checkPath(ref certFile))
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: certificate file not found: " + certFile;
                        throw e;
                    }
                    try
                    {
                        X509Certificate2 cert = new X509Certificate2(certFile, password);
                        certs_.Add(cert);
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
                Ice.PropertyDict certProps = properties.getPropertiesForPrefix(findPrefix);
                if(certProps.Count > 0)
                {
                    foreach(DictionaryEntry entry in certProps)
                    {
                        string name = (string)entry.Key;
                        string val = (string)entry.Value;
                        if(val.Length > 0)
                        {
                            string storeSpec = name.Substring(findPrefix.Length);
                            X509Certificate2Collection coll = findCertificates(name, storeSpec, val);
                            certs_.AddRange(coll);
                        }
                    }
                    if(certs_.Count == 0)
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: no certificates found";
                        throw e;
                    }
                }
            }

            initialized_ = true;
        }

        internal void setCertificates(X509Certificate2Collection certs)
        {
            if(initialized_)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: plugin is already initialized";
                throw e;
            }

            certs_ = certs;
        }

        internal void setCertificateVerifier(CertificateVerifier verifier)
        {
            verifier_ = verifier;
        }

        internal Ice.Communicator communicator()
        {
            return facade_.getCommunicator();
        }

        internal string defaultHost()
        {
            return facade_.getDefaultHost();
        }

        internal int networkTraceLevel()
        {
            return facade_.getNetworkTraceLevel();
        }

        internal string networkTraceCategory()
        {
            return facade_.getNetworkTraceCategory();
        }

        internal int securityTraceLevel()
        {
            return securityTraceLevel_;
        }

        internal string securityTraceCategory()
        {
            return securityTraceCategory_;
        }

        internal CertificateVerifier certificateVerifier()
        {
            return verifier_;
        }

        internal bool initialized()
        {
            return initialized_;
        }

        internal X509Certificate2Collection certs()
        {
            return certs_;
        }

        internal SslProtocols protocols()
        {
            return protocols_;
        }

        internal bool checkCRL()
        {
            return checkCRL_;
        }

        internal bool checkCertName()
        {
            return checkCertName_;
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
            communicator().getLogger().trace(securityTraceCategory_, s.ToString());
        }

        internal void verifyPeer(ConnectionInfo info, System.Net.Sockets.Socket fd, bool incoming)
        {
            if(verifyDepthMax_ > 0 && info.certs != null && info.certs.Length > verifyDepthMax_)
            {
                string msg = (incoming ? "incoming" : "outgoing") + " connection rejected:\n" +
                    "length of peer's certificate chain (" + info.certs.Length + ") exceeds maximum of " +
                    verifyDepthMax_ + "\n" +
                    IceInternal.Network.fdToString(fd);
                if(securityTraceLevel_ >= 1)
                {
                    logger_.trace(securityTraceCategory_, msg);
                }
                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = msg;
                throw ex;
            }

            if(!trustManager_.verify(info))
            {
                string msg = (incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                    IceInternal.Network.fdToString(fd);
                if(securityTraceLevel_ >= 1)
                {
                    logger_.trace(securityTraceCategory_, msg);
                }

                Ice.SecurityException ex = new Ice.SecurityException();
                ex.reason = "IceSSL: " + msg;
                throw ex;
            }

            if(verifier_ != null && !verifier_.verify(info))
            {
                string msg = (incoming ? "incoming" : "outgoing") + " connection rejected by certificate verifier\n" +
                    IceInternal.Network.fdToString(fd);
                if(securityTraceLevel_ >= 1)
                {
                    logger_.trace(securityTraceCategory_, msg);
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

            if(defaultDir_.Length > 0)
            {
                string s = defaultDir_ + Path.DirectorySeparatorChar + path;
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
            string password = null;
            if(arr.Length > 1)
            {
                password = arr[1];
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
            string val = communicator().getProperties().getProperty(property);
            if(val.Length > 0)
            {
                char[] delim = new char[] {',', ' '};
                string[] arr = val.Split(delim, StringSplitOptions.RemoveEmptyEntries);
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

        private Ice.Logger logger_;
        private IceInternal.ProtocolPluginFacade facade_;
        private int securityTraceLevel_;
        private string securityTraceCategory_;
        private bool initialized_;
        private string defaultDir_;
        private SslProtocols protocols_;
        private bool checkCertName_;
        private int verifyDepthMax_;
        private bool checkCRL_;
        private X509Certificate2Collection certs_;
        private CertificateVerifier verifier_;
        private TrustManager trustManager_;
    }
}
