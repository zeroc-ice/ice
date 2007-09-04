// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceSSL;

class Instance
{
    Instance(Ice.Communicator communicator)
    {
        _logger = communicator.getLogger();
        _facade = Ice.Util.getProtocolPluginFacade(communicator);
        _securityTraceLevel = communicator.getProperties().getPropertyAsIntWithDefault("IceSSL.Trace.Security", 0);
        _securityTraceCategory = "Security";
        _trustManager = new TrustManager(communicator);

        // 
        // Register the endpoint factory. We have to do this now, rather than
        // in initialize, because the communicator may need to interpret
        // proxies before the plugin is fully initialized.
        //
        _facade.addEndpointFactory(new EndpointFactoryI(this));
    }

    void
    initialize()
    {
        if(_initialized)
        {   
            return;
        }

        final String prefix = "IceSSL.";
        Ice.Properties properties = communicator().getProperties();

        //
        // Parse the cipher list.
        //
        String ciphers = properties.getProperty(prefix + "Ciphers");
        if(ciphers.length() > 0)
        {
            parseCiphers(ciphers);
        }

        //
        // Select protocols.
        //
        String protocols = properties.getProperty(prefix + "Protocols");
        if(protocols.length() > 0)
        {
            java.util.ArrayList<String> l = new java.util.ArrayList<String>();
            String[] arr = protocols.split("[ \t,]+");
            for(int i = 0; i < arr.length; ++i)
            {
                String s = arr[i].toLowerCase();
                if(s.equals("ssl3") || s.equals("sslv3"))
                {
                    l.add("SSLv3");
                }
                else if(s.equals("tls") || s.equals("tls1") || s.equals("tlsv1"))
                {
                    l.add("TLSv1");
                }
                else
                {
                    Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                    e.reason = "IceSSL: unrecognized protocol `" + arr[i] + "'";
                    throw e;
                }
            }
            _protocols = new String[l.size()];
            l.toArray(_protocols);
        }

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
        // If the user doesn't supply an SSLContext, we need to create one based
        // on property settings.
        //
        if(_context == null)
        {
            try
            {
                //
                // Check for a default directory. We look in this directory for
                // files mentioned in the configuration.
                //
                _defaultDir = properties.getProperty(prefix + "DefaultDir");

                //
                // We need a SecureRandom object.
                //
                // NOTE: The JDK recommends obtaining a SecureRandom object like this:
                //
                // java.security.SecureRandom rand = java.security.SecureRandom.getInstance("SHA1PRNG");
                //
                // However, there is a bug (6202721) which causes it to always use /dev/random,
                // which can lead to long delays at program startup. The workaround is to use
                // the default constructor.
                //
                java.security.SecureRandom rand = new java.security.SecureRandom();

                //
                // Check for seed data for the random number generator.
                //
                final String seedFiles = properties.getProperty(prefix + "Random");
                if(seedFiles.length() > 0)
                {
                    byte[] seed = null;
                    int start = 0;
                    final String[] arr = seedFiles.split(java.io.File.pathSeparator);
                    for(int i = 0; i < arr.length; ++i)
                    {
                        Ice.StringHolder seedFile = new Ice.StringHolder(arr[i]);
                        if(!checkPath(seedFile, false))
                        {
                            Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                            e.reason = "IceSSL: random seed file not found:\n" + arr[i];
                            throw e;
                        }
                        java.io.File f = new java.io.File(seedFile.value);
                        int num = (int)f.length();
                        if(seed == null)
                        {
                            seed = new byte[num];
                        }
                        else
                        {
                            byte[] tmp = new byte[seed.length + num];
                            System.arraycopy(seed, 0, tmp, 0, seed.length);
                            start = seed.length;
                            seed = tmp;
                        }
                        try
                        {
                            java.io.FileInputStream in = new java.io.FileInputStream(f);
                            in.read(seed, start, num);
                            in.close();
                        }
                        catch(java.io.IOException ex)
                        {
                            Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                            e.reason = "IceSSL: error while reading random seed file:\n" + arr[i];
                            e.initCause(ex);
                            throw e;
                        }
                    }
                    rand.setSeed(seed);
                }

                //
                // We call nextInt() in order to force the object to perform any time-consuming
                // initialization tasks now.
                //
                rand.nextInt();

                //
                // The keystore holds private keys and associated certificates.
                //
                Ice.StringHolder keystorePath = new Ice.StringHolder(properties.getProperty(prefix + "Keystore"));

                //
                // The password for the keys.
                //
                final String password = properties.getProperty(prefix + "Password");

                //
                // The password for the keystore.
                //
                final String keystorePassword = properties.getProperty(prefix + "KeystorePassword");

                //
                // The default keystore type value is "JKS", but it can also be "PKCS12".
                //
                final String defaultType = java.security.KeyStore.getDefaultType();
                final String keystoreType = properties.getPropertyWithDefault(prefix + "KeystoreType", defaultType);

                //
                // The alias of the key to use in authentication.
                //
                final String alias = properties.getProperty(prefix + "Alias");

                //
                // The truststore holds the certificates of trusted CAs.
                //
                Ice.StringHolder truststorePath = new Ice.StringHolder(properties.getProperty(prefix + "Truststore"));

                //
                // The password for the truststore.
                //
                final String truststorePassword = properties.getProperty(prefix + "TruststorePassword");

                //
                // The truststore type defaults to "JKS", but it can also be "PKCS12".
                //
                String truststoreType = properties.getPropertyWithDefault(prefix + "TruststoreType",
                                                                          java.security.KeyStore.getDefaultType());

                //
                // Collect the key managers.
                //
                javax.net.ssl.KeyManager[] keyManagers = null;
                if(keystorePath.value.length() > 0)
                {
                    if(!checkPath(keystorePath, false))
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: keystore file not found:\n" + keystorePath.value;
                        throw e;
                    }
                    java.security.KeyStore keys = java.security.KeyStore.getInstance(keystoreType);
                    try
                    {
                        char[] passwordChars = null;
                        if(keystorePassword.length() > 0)
                        {
                            passwordChars = keystorePassword.toCharArray();
                        }

                        java.io.BufferedInputStream bis =
                            new java.io.BufferedInputStream(new java.io.FileInputStream(keystorePath.value));
                        keys.load(bis, passwordChars);
                    }
                    catch(java.io.IOException ex)
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: unable to load keystore:\n" + keystorePath.value;
                        e.initCause(ex);
                        throw e;
                    }

                    String algorithm = javax.net.ssl.KeyManagerFactory.getDefaultAlgorithm();
                    javax.net.ssl.KeyManagerFactory kmf = javax.net.ssl.KeyManagerFactory.getInstance(algorithm);
                    kmf.init(keys, password.toCharArray());
                    keyManagers = kmf.getKeyManagers();

                    //
                    // If the user selected a specific alias, we need to wrap the key managers
                    // in order to return the desired alias.
                    //
                    if(alias.length() > 0)
                    {
                        if(!keys.isKeyEntry(alias))
                        {
                            Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                            e.reason = "IceSSL: keystore does not contain an entry with alias `" + alias + "'";
                            throw e;
                        }

                        for(int i = 0; i < keyManagers.length; ++i)
                        {
                            keyManagers[i] = new X509KeyManagerI((javax.net.ssl.X509KeyManager)keyManagers[i], alias);
                        }
                    }
                }

                //
                // Collect the trust managers.
                //
                javax.net.ssl.TrustManager[] trustManagers = null;
                if(truststorePath.value.length() > 0)
                {
                    if(!checkPath(truststorePath, false))
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: truststore file not found:\n" + truststorePath.value;
                        throw e;
                    }
                    java.security.KeyStore ts = java.security.KeyStore.getInstance(truststoreType);
                    try
                    {
                        char[] passwordChars = null;
                        if(truststorePassword.length() > 0)
                        {
                            passwordChars = truststorePassword.toCharArray();
                        }

                        java.io.BufferedInputStream bis =
                            new java.io.BufferedInputStream(new java.io.FileInputStream(truststorePath.value));
                        ts.load(bis, passwordChars);
                    }
                    catch(java.io.IOException ex)
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: unable to load truststore:\n" + truststorePath.value;
                        e.initCause(ex);
                        throw e;
                    }

                    String algorithm = javax.net.ssl.TrustManagerFactory.getDefaultAlgorithm();
                    javax.net.ssl.TrustManagerFactory tmf = javax.net.ssl.TrustManagerFactory.getInstance(algorithm);
                    tmf.init(ts);
                    trustManagers = tmf.getTrustManagers();
                }

                //
                // The default TrustManager implementation in IBM's JDK does not accept
                // anonymous ciphers, so we have to install our own.
                //
                if(trustManagers == null)
                {
                    trustManagers = new javax.net.ssl.TrustManager[1];
                    trustManagers[0] = new X509TrustManagerI(null);
                }
                else
                {
                    for(int i = 0; i < trustManagers.length; ++i)
                    {
                        trustManagers[i] = new X509TrustManagerI((javax.net.ssl.X509TrustManager)trustManagers[i]);
                    }
                }

                //
                // Initialize the SSL context.
                //
                _context = javax.net.ssl.SSLContext.getInstance("SSL");
                _context.init(keyManagers, trustManagers, rand);
            }
            catch(java.security.GeneralSecurityException ex)
            {
                Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                e.reason = "IceSSL: unable to initialize context";
                e.initCause(ex);
                throw e;
            }
        }

        _initialized = true;
    }

    void
    context(javax.net.ssl.SSLContext context)
    {
        if(_initialized)
        {
            Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
            ex.reason = "IceSSL: plugin is already initialized";
            throw ex;
        }

        _context = context;
    }

    javax.net.ssl.SSLContext
    context()
    {
        return _context;
    }

    void
    setCertificateVerifier(CertificateVerifier verifier)
    {
        _verifier = verifier;
    }

    Ice.Communicator
    communicator()
    {
        return _facade.getCommunicator();
    }

    String
    defaultHost()
    {
        return _facade.getDefaultHost();
    }

    int
    networkTraceLevel()
    {
        return _facade.getNetworkTraceLevel();
    }

    String
    networkTraceCategory()
    {
        return _facade.getNetworkTraceCategory();
    }

    int
    securityTraceLevel()
    {
        return _securityTraceLevel;
    }

    String
    securityTraceCategory()
    {
        return _securityTraceCategory;
    }

    boolean
    initialized()
    {
        return _initialized;
    }

    javax.net.ssl.SSLEngine
    createSSLEngine(boolean incoming)
    {
        javax.net.ssl.SSLEngine engine = _context.createSSLEngine();
        engine.setUseClientMode(!incoming);

        String[] cipherSuites = filterCiphers(engine.getSupportedCipherSuites(), engine.getEnabledCipherSuites());
        try
        {
            engine.setEnabledCipherSuites(cipherSuites);
        }
        catch(IllegalArgumentException ex)
        {
            Ice.SecurityException e = new Ice.SecurityException();
            e.reason = "IceSSL: invalid ciphersuite";
            e.initCause(ex);
            throw e;
        }

        if(_securityTraceLevel >= 1)
        {
            StringBuffer s = new StringBuffer();
            s.append("enabling SSL ciphersuites:");
            for(int i = 0; i < cipherSuites.length; ++i)
            {
                s.append("\n  " + cipherSuites[i]);
            }
            _logger.trace(_securityTraceCategory, s.toString());
        }

        if(_protocols != null)
        {
            try
            {
                engine.setEnabledProtocols(_protocols);
            }
            catch(IllegalArgumentException ex)
            {
                Ice.SecurityException e = new Ice.SecurityException();
                e.reason = "IceSSL: invalid protocol";
                e.initCause(ex);
                throw e;
            }
        }

        if(incoming)
        {
            int verifyPeer = communicator().getProperties().getPropertyAsIntWithDefault("IceSSL.VerifyPeer", 2);
            if(verifyPeer == 0)
            {
                engine.setWantClientAuth(false);
                engine.setNeedClientAuth(false);
            }
            else if(verifyPeer == 1)
            {
                engine.setWantClientAuth(true);
            }
            else
            {
                engine.setNeedClientAuth(true);
            }
        }

        try
        {
            engine.beginHandshake();
        }
        catch(javax.net.ssl.SSLException ex)
        {
            Ice.SecurityException e = new Ice.SecurityException();
            e.reason = "IceSSL: handshake error";
            e.initCause(ex);
            throw e;
        }

        return engine;
    }

    String[]
    filterCiphers(String[] supportedCiphers, String[] defaultCiphers)
    {
        java.util.LinkedList<String> result = new java.util.LinkedList<String>();
        if(_allCiphers)
        {
            for(int i = 0; i < supportedCiphers.length; ++i)
            {
                result.add(supportedCiphers[i]);
            }
        }
        else if(!_noCiphers)
        {
            for(int i = 0; i < defaultCiphers.length; ++i)
            {
                result.add(defaultCiphers[i]);
            }
        }

        if(_ciphers != null)
        {
            for(int i = 0; i < _ciphers.length; ++i)
            {
                CipherExpression ce = (CipherExpression)_ciphers[i];
                if(ce.not)
                {
                    java.util.Iterator e = result.iterator();
                    while(e.hasNext())
                    {
                        String cipher = (String)e.next();
                        if(ce.cipher != null)
                        {
                            if(ce.cipher.equals(cipher))
                            {
                                e.remove();
                            }
                        }
                        else
                        {
                            assert(ce.re != null);
                            java.util.regex.Matcher m = ce.re.matcher(cipher);
                            if(m.find())
                            {
                                e.remove();
                            }
                        }
                    }
                }
                else
                {
                    if(ce.cipher != null)
                    {
                        result.add(0, ce.cipher);
                    }
                    else
                    {
                        assert(ce.re != null);
                        for(int j = 0; j < supportedCiphers.length; ++j)
                        {
                            java.util.regex.Matcher m = ce.re.matcher(supportedCiphers[j]);
                            if(m.find())
                            {
                                result.add(0, supportedCiphers[j]);
                            }
                        }
                    }
                }
            }
        }

        String[] arr = new String[result.size()];
        result.toArray(arr);
        return arr;
    }

    String[]
    protocols()
    {
        return _protocols;
    }

    void
    traceConnection(java.nio.channels.SocketChannel fd, javax.net.ssl.SSLEngine engine, boolean incoming)
    {
        javax.net.ssl.SSLSession session = engine.getSession();
        String msg = "SSL summary for " + (incoming ? "incoming" : "outgoing") + " connection\n" +
            "cipher = " + session.getCipherSuite() + "\n" +
            "protocol = " + session.getProtocol() + "\n" +
            IceInternal.Network.fdToString(fd);
        _logger.trace(_securityTraceCategory, msg);
    }

    void
    verifyPeer(ConnectionInfo info, java.nio.channels.SelectableChannel fd, String address, boolean incoming)
    {
        if(_verifyDepthMax > 0 && info.certs != null && info.certs.length > _verifyDepthMax)
        {
            String msg = (incoming ? "incoming" : "outgoing") + " connection rejected:\n" +
                "length of peer's certificate chain (" + info.certs.length + ") exceeds maximum of " +
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

        //
        // Extract the IP addresses and the DNS names from the subject
        // alternative names.
        //
        if(info.certs != null)
        {
            try
            {
                java.util.Collection subjectAltNames =
                    ((java.security.cert.X509Certificate)info.certs[0]).getSubjectAlternativeNames();
                java.util.ArrayList<String> ipAddresses = new java.util.ArrayList<String>();
                java.util.ArrayList<String> dnsNames = new java.util.ArrayList<String>();
                if(subjectAltNames != null)
                {
                    java.util.Iterator i = subjectAltNames.iterator();
                    while(i.hasNext())
                    {
                        java.util.List l = (java.util.List)i.next();
                        assert(!l.isEmpty());
                        Integer n = (Integer)l.get(0);
                        if(n.intValue() == 7)
                        {
                            ipAddresses.add((String)l.get(1));
                        }
                        else if(n.intValue() == 2)
                        {
                            dnsNames.add(((String)l.get(1)).toLowerCase());
                        }
                    }
                }

                //
                // Compare the peer's address against the dnsName and ipAddress values.
                // This is only relevant for an outgoing connection.
                //
                if(address.length() > 0)
                {
                    boolean certNameOK = ipAddresses.contains(address);
                    if(!certNameOK)
                    {
                        certNameOK = dnsNames.contains(address.toLowerCase());
                    }

                    //
                    // Log a message if the name comparison fails. If CheckCertName is defined,
                    // we also raise an exception to abort the connection. Don't log a message if
                    // CheckCertName is not defined and a verifier is present.
                    //
                    if(!certNameOK && (_checkCertName || (_securityTraceLevel >= 1 && _verifier == null)))
                    {
                        StringBuffer sb = new StringBuffer();
                        sb.append("IceSSL: ");
                        if(!_checkCertName)
                        {
                            sb.append("ignoring ");
                        }
                        sb.append("certificate validation failure:\npeer certificate does not contain `" +
                                  address + "' in its subjectAltName extension");
                        if(!dnsNames.isEmpty())
                        {
                            sb.append("\nDNS names found in certificate: ");
                            for(int j = 0; j < dnsNames.size(); ++j)
                            {
                                if(j > 0)
                                {
                                    sb.append(", ");
                                }
                                sb.append(dnsNames.get(j).toString());
                            }
                        }
                        if(!ipAddresses.isEmpty())
                        {
                            sb.append("\nIP addresses found in certificate: ");
                            for(int j = 0; j < ipAddresses.size(); ++j)
                            {
                                if(j > 0)
                                {
                                    sb.append(", ");
                                }
                                sb.append(ipAddresses.get(j).toString());
                            }
                        }
                        if(_securityTraceLevel >= 1)
                        {
                            _logger.trace(_securityTraceCategory, sb.toString());
                        }
                        if(_checkCertName)
                        {
                            Ice.SecurityException ex = new Ice.SecurityException();
                            ex.reason = sb.toString();
                            throw ex;
                        }
                    }
                }
            }
            catch(java.security.cert.CertificateParsingException ex)
            {
                assert(false);
            }
        }

        if(!_trustManager.verify(info))
        {
            String msg = (incoming ? "incoming" : "outgoing") + " connection rejected by trust manager\n" +
                IceInternal.Network.fdToString(fd);
            if(_securityTraceLevel >= 1)
            {
                _logger.trace(_securityTraceCategory, msg);
            }
            Ice.SecurityException ex = new Ice.SecurityException();
            ex.reason = msg;
            throw ex;
        }

        if(_verifier != null && !_verifier.verify(info))
        {
            String msg = (incoming ? "incoming" : "outgoing") + " connection rejected by certificate verifier\n" +
            IceInternal.Network.fdToString(fd);
            
            if(_securityTraceLevel > 0)
            {
                _logger.trace(_securityTraceCategory, msg);
            }
            
            Ice.SecurityException ex = new Ice.SecurityException();
            ex.reason = msg;
            throw ex;
        }
    }

    private void
    parseCiphers(String ciphers)
    {
        java.util.ArrayList<CipherExpression> cipherList = new java.util.ArrayList<CipherExpression>();
        String[] expr = ciphers.split("[ \t]+");
        for(int i = 0; i < expr.length; ++i)
        {
            if(expr[i].equals("ALL"))
            {
                if(i != 0)
                {
                    Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                    ex.reason = "IceSSL: `ALL' must be first in cipher list `" + ciphers + "'";
                    throw ex;
                }
                _allCiphers = true;
            }
            else if(expr[i].equals("NONE"))
            {
                if(i != 0)
                {
                    Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                    ex.reason = "IceSSL: `NONE' must be first in cipher list `" + ciphers + "'";
                    throw ex;
                }
                _noCiphers = true;
            }
            else
            {
                CipherExpression ce = new CipherExpression();
                String exp = expr[i];
                if(exp.charAt(0) == '!')
                {
                    ce.not = true;
                    if(exp.length() > 1)
                    {
                        exp = exp.substring(1);
                    }
                    else
                    {
                        Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                        ex.reason = "IceSSL: invalid cipher expression `" + exp + "'";
                        throw ex;
                    }
                }

                if(exp.charAt(0) == '(')
                {
                    if(!exp.endsWith(")"))
                    {
                        Ice.PluginInitializationException ex = new Ice.PluginInitializationException();
                        ex.reason = "IceSSL: invalid cipher expression `" + exp + "'";
                        throw ex;
                    }

                    try
                    {
                        ce.re = java.util.regex.Pattern.compile(exp.substring(1, exp.length() - 2));
                    }
                    catch(java.util.regex.PatternSyntaxException ex)
                    {
                        Ice.PluginInitializationException e = new Ice.PluginInitializationException();
                        e.reason = "IceSSL: invalid cipher expression `" + exp + "'";
                        e.initCause(ex);
                        throw e;
                    }
                }
                else
                {
                    ce.cipher = exp;
                }

                cipherList.add(ce);
            }
        }
        _ciphers = new CipherExpression[cipherList.size()];
        cipherList.toArray(_ciphers);
    }

    private boolean
    checkPath(Ice.StringHolder path, boolean dir)
    {
        //
        // Check if file exists. If not, try prepending the default
        // directory and check again. If the file is found, the
        // string argument is modified and true is returned. Otherwise
        // false is returned.
        //
        java.io.File f = new java.io.File(path.value);
        if(f.exists())
        {
            return dir ? f.isDirectory() : f.isFile();
        }

        if(_defaultDir.length() > 0)
        {
            String s = _defaultDir + java.io.File.separator + path.value;
            f = new java.io.File(s);
            if(f.exists() && ((!dir && f.isFile()) || (dir && f.isDirectory())))
            {
                path.value = s;
                return true;
            }
        }

        return false;
    }

    private static class CipherExpression
    {
        boolean not;
        String cipher;
        java.util.regex.Pattern re;
    }

    private Ice.Logger _logger;
    private IceInternal.ProtocolPluginFacade _facade;
    private int _securityTraceLevel;
    private String _securityTraceCategory;
    private boolean _initialized;
    private javax.net.ssl.SSLContext _context;
    private String _defaultDir;
    private CipherExpression[] _ciphers;
    private boolean _allCiphers;
    private boolean _noCiphers;
    private String[] _protocols;
    private boolean _checkCertName;
    private int _verifyDepthMax;
    private CertificateVerifier _verifier;
    private TrustManager _trustManager;
}
