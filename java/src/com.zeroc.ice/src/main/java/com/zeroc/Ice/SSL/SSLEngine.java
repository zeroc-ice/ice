// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice.SSL;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.InitializationException;
import com.zeroc.Ice.Logger;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.SecurityException;
import com.zeroc.Ice.Util;

import java.io.BufferedInputStream;
import java.io.File;
import java.io.IOException;
import java.io.InputStream;
import java.io.PrintWriter;
import java.io.StringWriter;
import java.net.MalformedURLException;
import java.net.URL;
import java.security.GeneralSecurityException;
import java.security.KeyStore;
import java.security.cert.CertificateException;
import java.security.cert.X509Certificate;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Enumeration;
import java.util.List;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SNIHostName;
import javax.net.ssl.SNIServerName;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLParameters;
import javax.net.ssl.SSLSession;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509ExtendedKeyManager;
import javax.net.ssl.X509TrustManager;

public class SSLEngine {
    public SSLEngine(Communicator communicator) {
        _communicator = communicator;
        _logger = _communicator.getLogger();
        _securityTraceLevel =
            _communicator.getProperties().getIcePropertyAsInt("IceSSL.Trace.Security");
        _securityTraceCategory = "Security";
        _trustManager = new TrustManager(_communicator);
    }

    public void initialize() {
        Properties properties = communicator().getProperties();

        //
        // CheckCertName determines whether we compare the name in a peer's certificate against its
        // hostname.
        //
        _checkCertName = properties.getIcePropertyAsInt("IceSSL.CheckCertName") > 0;

        //
        // CheckCertName > 1 enables SNI, the SNI extension applies to client connections,
        // indicating the hostname to the server (must be DNS hostname, not an IP address).
        //
        _serverNameIndication = properties.getIcePropertyAsInt("IceSSL.CheckCertName") > 1;

        //
        // VerifyPeer determines whether certificate validation failures abort a connection.
        //
        _verifyPeer = properties.getIcePropertyAsInt("IceSSL.VerifyPeer");

        //
        // If the user doesn't supply an SSLContext, we need to create one based on property
        // settings.
        //
        if (_context == null) {
            try {
                //
                // Check for a default directory. We look in this directory for files mentioned in
                // the configuration.
                //
                _defaultDir = properties.getIceProperty("IceSSL.DefaultDir");

                //
                // The keystore holds private keys and associated certificates.
                //
                String keystorePath = properties.getIceProperty("IceSSL.Keystore");

                //
                // The password for the keys.
                //
                String password = properties.getIceProperty("IceSSL.Password");

                //
                // The password for the keystore.
                //
                String keystorePassword = properties.getIceProperty("IceSSL.KeystorePassword");

                //
                // The default keystore type is usually "JKS", but the legal values are determined
                // by the JVM implementation. Other possibilities include "PKCS12" and "BKS".
                //
                final String defaultType = KeyStore.getDefaultType();
                final String keystoreType =
                    properties.getPropertyWithDefault("IceSSL.KeystoreType", defaultType);

                //
                // The alias of the key to use in authentication.
                //
                String alias = properties.getIceProperty("IceSSL.Alias");
                boolean overrideAlias = !alias.isEmpty(); // Always use the configured alias

                //
                // The truststore holds the certificates of trusted CAs.
                //
                String truststorePath = properties.getIceProperty("IceSSL.Truststore");

                //
                // The password for the truststore.
                //
                String truststorePassword = properties.getIceProperty("IceSSL.TruststorePassword");

                //
                // The default truststore type is usually "JKS", but the legal values are determined
                // by the JVM implementation. Other possibilities include "PKCS12" and "BKS".
                //
                final String truststoreType =
                    properties.getPropertyWithDefault("IceSSL.TruststoreType", defaultType);

                //
                // Collect the key managers.
                //
                KeyManager[] keyManagers = null;
                KeyStore keys = null;
                if (!keystorePath.isEmpty()) {
                    InputStream keystoreStream = null;
                    try {
                        keystoreStream = openResource(keystorePath);
                        if (keystoreStream == null) {
                            throw new InitializationException(
                                "SSL transport: keystore not found:\n" + keystorePath);
                        }

                        keys = KeyStore.getInstance(keystoreType);
                        char[] passwordChars = null;
                        if (!keystorePassword.isEmpty()) {
                            passwordChars = keystorePassword.toCharArray();
                        } else if ("BKS".equals(keystoreType) || "PKCS12".equals(keystoreType)) {
                            // Bouncy Castle or PKCS12 does not permit null passwords.
                            passwordChars = new char[0];
                        }

                        keys.load(keystoreStream, passwordChars);

                        if (passwordChars != null) {
                            Arrays.fill(passwordChars, '\0');
                        }
                        keystorePassword = null;
                    } catch (IOException ex) {
                        throw new InitializationException(
                            "SSL transport: unable to load keystore:\n" + keystorePath, ex);
                    } finally {
                        if (keystoreStream != null) {
                            try {
                                keystoreStream.close();
                            } catch (IOException e) {
                                // Ignore.
                            }
                        }
                    }

                    String algorithm = KeyManagerFactory.getDefaultAlgorithm();
                    KeyManagerFactory kmf =
                        KeyManagerFactory.getInstance(algorithm);
                    char[] passwordChars = new char[0]; // This password cannot be null.
                    if (!password.isEmpty()) {
                        passwordChars = password.toCharArray();
                    }
                    kmf.init(keys, passwordChars);
                    if (passwordChars.length > 0) {
                        Arrays.fill(passwordChars, '\0');
                    }
                    password = null;
                    keyManagers = kmf.getKeyManagers();

                    //
                    // If no alias is specified, we look for the first key entry in the key store.
                    //
                    // This is required to force the key manager to always choose a certificate
                    // even if there's no certificate signed by any of the CA names sent by the
                    // server. Ice servers might indeed not always send the CA names of their
                    // trusted roots.
                    //
                    if (alias.isEmpty()) {
                        for (Enumeration<String> e = keys.aliases();
                                e.hasMoreElements(); ) {
                            String a = e.nextElement();
                            if (keys.isKeyEntry(a)) {
                                alias = a;
                                break;
                            }
                        }
                    }

                    if (!alias.isEmpty()) {
                        //
                        // If the user selected a specific alias, we need to wrap the key managers
                        // in order to return the desired alias.
                        //
                        if (!keys.isKeyEntry(alias)) {
                            throw new InitializationException(
                                "SSL transport: keystore does not contain an entry with alias `"
                                    + alias
                                    + "'");
                        }

                        for (int i = 0; i < keyManagers.length; i++) {
                            keyManagers[i] =
                                new X509KeyManagerI(
                                    (X509ExtendedKeyManager) keyManagers[i],
                                    alias,
                                    overrideAlias);
                        }
                    }
                }

                //
                // Load the truststore.
                //
                KeyStore ts = null;
                if (!truststorePath.isEmpty()) {
                    //
                    // If the trust store and the key store are the same file, don't
                    // create another key store.
                    //
                    if (truststorePath.equals(keystorePath)) {
                        assert keys != null;
                        ts = keys;
                    } else {
                        InputStream truststoreStream = null;
                        try {
                            truststoreStream = openResource(truststorePath);
                            if (truststoreStream == null) {
                                throw new InitializationException(
                                    "SSL transport: truststore not found:\n"
                                        + truststorePath);
                            }

                            ts = KeyStore.getInstance(truststoreType);

                            char[] passwordChars = null;
                            if (!truststorePassword.isEmpty()) {
                                passwordChars = truststorePassword.toCharArray();
                            } else if ("BKS".equals(truststoreType)
                                || "PKCS12".equals(truststoreType)) {
                                // Bouncy Castle or PKCS12 does not permit null passwords.
                                passwordChars = new char[0];
                            }

                            ts.load(truststoreStream, passwordChars);

                            if (passwordChars != null) {
                                Arrays.fill(passwordChars, '\0');
                            }
                            truststorePassword = null;
                        } catch (IOException ex) {
                            throw new InitializationException(
                                "SSL transport: unable to load truststore:\n" + truststorePath,
                                ex);
                        } finally {
                            if (truststoreStream != null) {
                                try {
                                    truststoreStream.close();
                                } catch (IOException e) {
                                    // Ignore.
                                }
                            }
                        }
                    }
                }

                //
                // Collect the trust managers. Use IceSSL.Truststore if specified, otherwise use the
                // Java root CAs if Ice.Use.PlatformCAs is enabled. If none of these are enabled,
                // use the keystore or a dummy trust manager which rejects any certificate.
                //
                javax.net.ssl.TrustManager[] trustManagers = null;
                {
                    String algorithm = TrustManagerFactory.getDefaultAlgorithm();
                    TrustManagerFactory tmf =
                        TrustManagerFactory.getInstance(algorithm);
                    KeyStore trustStore = null;
                    if (ts != null) {
                        trustStore = ts;
                    } else if (properties.getIcePropertyAsInt("IceSSL.UsePlatformCAs") <= 0) {
                        if (keys != null) {
                            trustStore = keys;
                        } else {
                            trustManagers =
                                new javax.net.ssl.TrustManager[]{
                                    new X509TrustManager() {
                                        @Override
                                        public void checkClientTrusted(
                                                    X509Certificate[] chain, String authType)
                                            throws CertificateException {
                                            throw new CertificateException("no trust anchors");
                                        }

                                        @Override
                                        public void checkServerTrusted(
                                                    X509Certificate[] chain, String authType)
                                            throws CertificateException {
                                            throw new CertificateException("no trust anchors");
                                        }

                                        @Override
                                        public X509Certificate[] getAcceptedIssuers() {
                                            return new X509Certificate[0];
                                        }
                                    }
                                };
                        }
                    } else {
                        trustStore = null;
                    }

                    //
                    // Attempting to establish an outgoing connection with an empty truststore can
                    // cause hangs that eventually result in an exception such as:
                    //
                    // java.security.InvalidAlgorithmParameterException: the trustAnchors parameter
                    // must be non-empty
                    //
                    if (trustStore != null && trustStore.size() == 0) {
                        throw new InitializationException("SSL transport: truststore is empty");
                    }

                    if (trustManagers == null) {
                        tmf.init(trustStore);
                        trustManagers = tmf.getTrustManagers();
                    }
                    assert (trustManagers != null);
                }

                //
                // Initialize the SSL context.
                //
                _context = SSLContext.getInstance("TLS");
                _context.init(keyManagers, trustManagers, null);
            } catch (GeneralSecurityException ex) {
                throw new InitializationException(
                    "SSL transport: unable to initialize context", ex);
            }
        }

        //
        // Clear cached input streams.
        //
        _keystoreStream = null;
        _truststoreStream = null;
    }

    int securityTraceLevel() {
        return _securityTraceLevel;
    }

    String securityTraceCategory() {
        return _securityTraceCategory;
    }

    javax.net.ssl.SSLEngine createSSLEngine(boolean incoming, String host, int port) {
        javax.net.ssl.SSLEngine engine;
        try {
            if (host != null) {
                engine = _context.createSSLEngine(host, port);
            } else {
                engine = _context.createSSLEngine();
            }
            engine.setUseClientMode(!incoming);
        } catch (Exception ex) {
            throw new SecurityException(
                "SSL transport: couldn't create SSL engine", ex);
        }

        if (incoming) {
            if (_verifyPeer == 0) {
                engine.setWantClientAuth(false);
                engine.setNeedClientAuth(false);
            } else if (_verifyPeer == 1) {
                engine.setWantClientAuth(true);
            } else {
                engine.setNeedClientAuth(true);
            }
        } else {
            //
            // Enable the HTTPS hostname verification algorithm
            //
            if (_checkCertName && _verifyPeer > 0 && host != null) {
                SSLParameters params = new SSLParameters();
                params.setEndpointIdentificationAlgorithm("HTTPS");
                engine.setSSLParameters(params);
            }
        }

        // Server name indication
        if (!incoming && _serverNameIndication) {
            SNIHostName serverName = null;
            try {
                serverName = new SNIHostName(host);
                SSLParameters sslParams = engine.getSSLParameters();
                List<SNIServerName> serverNames = new ArrayList<>();
                serverNames.add(serverName);
                sslParams.setServerNames(serverNames);
                engine.setSSLParameters(sslParams);
            } catch (IllegalArgumentException ex) {
                // Invalid SNI hostname, ignore because it might be an IP
            }
        }

        return engine;
    }

    void traceConnection(String desc, javax.net.ssl.SSLEngine engine, boolean incoming) {
        SSLSession session = engine.getSession();
        String msg =
            "SSL summary for "
                + (incoming ? "incoming" : "outgoing")
                + " connection\n"
                + "cipher = "
                + session.getCipherSuite()
                + "\n"
                + "protocol = "
                + session.getProtocol()
                + "\n"
                + desc;
        _logger.trace(_securityTraceCategory, msg);
    }

    Communicator communicator() {
        return _communicator;
    }

    void verifyPeer(String address, ConnectionInfo info, String desc) {
        //
        // IceSSL.VerifyPeer is translated into the proper SSLEngine configuration
        // for a server, but we have to do it ourselves for a client.
        //
        if (!info.incoming) {
            if (_verifyPeer > 0 && !info.verified) {
                throw new SecurityException(
                    "SSL transport: server did not supply a certificate");
            }
        }

        if (!_trustManager.verify(info, desc)) {
            String msg =
                (info.incoming ? "incoming" : "outgoing")
                    + " connection rejected by trust manager\n"
                    + desc;
            if (_securityTraceLevel >= 1) {
                _logger.trace(_securityTraceCategory, msg);
            }
            throw new SecurityException(msg);
        }
    }

    void trustManagerFailure(boolean incoming, CertificateException ex)
        throws CertificateException {
        if (_verifyPeer == 0) {
            if (_securityTraceLevel >= 1) {
                String msg = "ignoring peer verification failure";
                if (_securityTraceLevel > 1) {
                    StringWriter sw = new StringWriter();
                    PrintWriter pw = new PrintWriter(sw);
                    ex.printStackTrace(pw);
                    pw.flush();
                    msg += ":\n" + sw.toString();
                }
                _logger.trace(_securityTraceCategory, msg);
            }
        } else {
            throw ex;
        }
    }

    @SuppressWarnings("deprecation")
    private InputStream openResource(String path) throws IOException {
        boolean isAbsolute = false;
        try {
            new URL(path);
            isAbsolute = true;
        } catch (MalformedURLException ex) {
            File f = new File(path);
            isAbsolute = f.isAbsolute();
        }

        InitializationData initData =
            _communicator.getInstance().initializationData();

        ClassLoader classLoader =
            initData.classLoader != null ? initData.classLoader : getClass().getClassLoader();

        InputStream stream = Util.openResource(classLoader, path);

        //
        // If the first attempt fails and IceSSL.DefaultDir is defined and the original
        // path is relative, we prepend the default directory and try again.
        //
        if (stream == null && !_defaultDir.isEmpty() && !isAbsolute) {
            stream =
                Util.openResource(
                    classLoader, _defaultDir + File.separator + path);
        }

        if (stream != null) {
            stream = new BufferedInputStream(stream);
        }

        return stream;
    }

    private final Communicator _communicator;
    private final Logger _logger;
    private final int _securityTraceLevel;
    private final String _securityTraceCategory;
    private SSLContext _context;
    private String _defaultDir;
    private boolean _checkCertName;
    private boolean _serverNameIndication;
    private int _verifyPeer;
    private final TrustManager _trustManager;
    private InputStream _keystoreStream;
    private InputStream _truststoreStream;
}
