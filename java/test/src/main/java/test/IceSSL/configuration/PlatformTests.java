// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.SecurityException;
import com.zeroc.Ice.Util;

import test.IceSSL.configuration.Test.ServerPrx;
import test.TestHelper;

import java.io.FileInputStream;
import java.io.PrintWriter;
import java.net.Socket;
import java.security.KeyManagementException;
import java.security.KeyStore;
import java.security.NoSuchAlgorithmException;
import java.security.Principal;
import java.security.PrivateKey;
import java.security.cert.X509Certificate;

import javax.net.ssl.KeyManager;
import javax.net.ssl.KeyManagerFactory;
import javax.net.ssl.SSLContext;
import javax.net.ssl.SSLEngine;
import javax.net.ssl.TrustManager;
import javax.net.ssl.TrustManagerFactory;
import javax.net.ssl.X509ExtendedKeyManager;

public class PlatformTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    public static KeyManagerFactory loadKeyManagerFactory(String keyStorePath) {
        try {
            KeyStore keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
            try (FileInputStream input = new FileInputStream(keyStorePath)) {
                keyStore.load(input, "password".toCharArray());
            }
            KeyManagerFactory keyManagerFactory =
                KeyManagerFactory.getInstance(KeyManagerFactory.getDefaultAlgorithm());
            keyManagerFactory.init(keyStore, "password".toCharArray());
            return keyManagerFactory;
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static TrustManagerFactory loadTrustManagerFactory(String trustStorePath) {
        try {
            KeyStore keyStore = KeyStore.getInstance(KeyStore.getDefaultType());
            try (FileInputStream input = new FileInputStream(trustStorePath)) {
                keyStore.load(input, "password".toCharArray());
            }
            TrustManagerFactory trustManagerFactory =
                TrustManagerFactory.getInstance(TrustManagerFactory.getDefaultAlgorithm());
            trustManagerFactory.init(keyStore);
            return trustManagerFactory;
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static Communicator createServer(
            TestHelper helper, String keyStorePath, String trustStorePath)
        throws NoSuchAlgorithmException, KeyManagementException {
        var sslContext = SSLContext.getInstance("TLS");

        KeyManagerFactory keyManagerFactory =
            keyStorePath == null ? null : loadKeyManagerFactory(keyStorePath);
        TrustManagerFactory trustManagerFactory =
            trustStorePath == null ? null : loadTrustManagerFactory(trustStorePath);
        boolean clientCertificateRequired = trustStorePath != null;

        KeyManager[] keyManagers =
            keyManagerFactory == null ? null : keyManagerFactory.getKeyManagers();
        TrustManager[] trustManagers =
            trustManagerFactory == null ? null : trustManagerFactory.getTrustManagers();
        sslContext.init(keyManagers, trustManagers, null);

        var communicator = Util.initialize();
        var adapter =
            communicator.createObjectAdapterWithEndpoints(
                "ServerAdapter",
                helper.getTestEndpoint(10, "ssl"),
                (String peerHost, int peerPort) -> {
                    var engine = sslContext.createSSLEngine(peerHost, peerPort);
                    engine.setNeedClientAuth(clientCertificateRequired);
                    return engine;
                });
        adapter.add(new ServerI(communicator), Util.stringToIdentity("server"));
        adapter.activate();
        return communicator;
    }

    public static Communicator createClient(
            TestHelper helper, String keyStorePath, String trustStorePath)
        throws NoSuchAlgorithmException, KeyManagementException {
        var sslContext = SSLContext.getInstance("TLS");

        KeyManagerFactory keyManagerFactory =
            keyStorePath == null ? null : loadKeyManagerFactory(keyStorePath);
        TrustManagerFactory trustManagerFactory =
            trustStorePath == null ? null : loadTrustManagerFactory(trustStorePath);

        KeyManager[] keyManagers =
            keyManagerFactory == null ? null : keyManagerFactory.getKeyManagers();
        TrustManager[] trustManagers =
            trustManagerFactory == null ? null : trustManagerFactory.getTrustManagers();
        sslContext.init(keyManagers, trustManagers, null);

        var initializationData = new InitializationData();
        initializationData.clientSSLEngineFactory =
            (String peerHost, int peerPort) -> sslContext.createSSLEngine(peerHost, peerPort);
        return Util.initialize(initializationData);
    }

    public static void clientValidatesServerUsingTrustStore(
            TestHelper helper, String certificatesPath) {
        try {
            PrintWriter out = helper.getWriter();
            out.print("client validates server certificate using trust store... ");
            out.flush();

            try (var serverCommunicator =
                createServer(helper, certificatesPath + "/s_rsa_ca1.jks", null)) {
                try (var clientCommunicator =
                    createClient(helper, null, certificatesPath + "/cacert1.jks")) {
                    var obj =
                        ServerPrx.createProxy(
                            clientCommunicator,
                            "server:" + helper.getTestEndpoint(10, "ssl"));
                    obj.ice_ping();
                }
            }

            out.println("ok");
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static void clientValidatesServerUsingDefaultTrustStore(
            TestHelper helper, String certificatesPath) {
        try {
            PrintWriter out = helper.getWriter();
            out.print("client validates server certificate using default trust store... ");
            out.flush();

            try (var clientCommunicator = createClient(helper, null, null)) {
                var obj =
                    ObjectPrx.createProxy(
                        clientCommunicator,
                        "Glacier2/router:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
                obj.ice_ping();
            }
            out.println("ok");
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static void clientRejectsServerUsingTrustStore(
            TestHelper helper, String certificatesPath) {
        try {
            PrintWriter out = helper.getWriter();
            out.print("client rejects server certificate using trust store... ");
            out.flush();

            try (var serverCommunicator =
                createServer(helper, certificatesPath + "/s_rsa_ca1.jks", null)) {
                try (var clientCommunicator =
                    createClient(helper, null, certificatesPath + "/cacert2.jks")) {
                    var obj =
                        ServerPrx.createProxy(
                            clientCommunicator,
                            "server:" + helper.getTestEndpoint(10, "ssl"));

                    try {
                        obj.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    }
                }
            }
            out.println("ok");
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static void serverValidatesClientUsingTrustStore(
            TestHelper helper, String certificatesPath) {
        try {
            PrintWriter out = helper.getWriter();
            out.print("server validates client certificate using trust store... ");
            out.flush();

            var serverCertificatePath = certificatesPath + "/s_rsa_ca1.jks";
            var clientCertificatePath = certificatesPath + "/c_rsa_ca1.jks";
            var trustedStorePath = certificatesPath + "/cacert1.jks";

            try (var serverCommunicator =
                createServer(helper, serverCertificatePath, trustedStorePath)) {
                try (var clientCommunicator =
                    createClient(helper, clientCertificatePath, trustedStorePath)) {
                    var obj =
                        ServerPrx.createProxy(
                            clientCommunicator,
                            "server:" + helper.getTestEndpoint(10, "ssl"));
                    obj.ice_ping();
                }
            }

            out.println("ok");
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static void serverRejectsClientUsingTrustStore(
            TestHelper helper, String certificatesPath) {
        try {
            PrintWriter out = helper.getWriter();
            out.print("server rejects client certificate using trust store... ");
            out.flush();

            var serverCertificatePath = certificatesPath + "/s_rsa_ca1.jks";
            var clientCertificatePath = certificatesPath + "/c_rsa_ca2.jks";
            var trustedStorePath = certificatesPath + "/cacert1.jks";

            try (var serverCommunicator =
                createServer(helper, serverCertificatePath, trustedStorePath)) {
                try (var clientCommunicator =
                    createClient(helper, clientCertificatePath, trustedStorePath)) {
                    var obj =
                        ServerPrx.createProxy(
                            clientCommunicator,
                            "server:" + helper.getTestEndpoint(10, "ssl"));

                    try {
                        obj.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    } catch (ConnectionLostException ex) {
                        // Expected
                    }
                }
            }
            out.println("ok");
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static void serverHotCertificateReload(TestHelper helper, String certificatesPath) {
        PrintWriter out = helper.getWriter();
        out.print("server hot certificate reload... ");
        out.flush();

        class ReloadableKeyManager extends X509ExtendedKeyManager {
            private X509ExtendedKeyManager _decoratee;

            public ReloadableKeyManager(String keyStorePath) {
                var keyManagerFactory = loadKeyManagerFactory(keyStorePath);
                _decoratee = (X509ExtendedKeyManager) keyManagerFactory.getKeyManagers()[0];
            }

            public void reload(String keyStorePath) {
                var keyManagerFactory = loadKeyManagerFactory(keyStorePath);
                _decoratee = (X509ExtendedKeyManager) keyManagerFactory.getKeyManagers()[0];
            }

            @Override
            public String chooseClientAlias(
                    String[] keyType, Principal[] issuers, Socket socket) {
                return _decoratee.chooseClientAlias(keyType, issuers, socket);
            }

            @Override
            public String chooseEngineClientAlias(
                    String[] keyType,
                    Principal[] issuers,
                    SSLEngine engine) {
                return _decoratee.chooseEngineClientAlias(keyType, issuers, engine);
            }

            @Override
            public String chooseServerAlias(
                    String keyType, Principal[] issuers, Socket socket) {
                return _decoratee.chooseServerAlias(keyType, issuers, socket);
            }

            @Override
            public String chooseEngineServerAlias(
                    String keyType,
                    Principal[] issuers,
                    SSLEngine engine) {
                return _decoratee.chooseEngineServerAlias(keyType, issuers, engine);
            }

            @Override
            public X509Certificate[] getCertificateChain(String alias) {
                return _decoratee.getCertificateChain(alias);
            }

            @Override
            public String[] getClientAliases(String keyType, Principal[] issuers) {
                return _decoratee.getClientAliases(keyType, issuers);
            }

            @Override
            public String[] getServerAliases(String keyType, Principal[] issuers) {
                return _decoratee.getServerAliases(keyType, issuers);
            }

            @Override
            public PrivateKey getPrivateKey(String alias) {
                return _decoratee.getPrivateKey(alias);
            }
        }

        try (var serverCommunicator = Util.initialize()) {
            var keyManager = new ReloadableKeyManager(certificatesPath + "/s_rsa_ca1.jks");
            var sslContext = SSLContext.getInstance("TLS");
            sslContext.init(new KeyManager[]{keyManager}, null, null);
            var adapter =
                serverCommunicator.createObjectAdapterWithEndpoints(
                    "ServerAdapter",
                    helper.getTestEndpoint(10, "ssl"),
                    (String peerHost, int peerPort) -> {
                        return sslContext.createSSLEngine(peerHost, peerPort);
                    });
            adapter.add(
                new ServerI(serverCommunicator), Util.stringToIdentity("server"));
            adapter.activate();

            try (var clientCommunicator =
                createClient(helper, null, certificatesPath + "/cacert1.jks")) {
                var obj =
                    ServerPrx.createProxy(
                        clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
                obj.ice_ping();
            }

            // CA2 is not accepted with the initial configuration
            try (var clientCommunicator =
                createClient(helper, null, certificatesPath + "/cacert2.jks")) {
                var obj =
                    ServerPrx.createProxy(
                        clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
                try {
                    obj.ice_ping();
                    test(false);
                } catch (SecurityException ex) {
                    // Expected
                }
            }

            keyManager.reload(certificatesPath + "/s_rsa_ca2.jks");

            // CA2 is accepted with the new configuration
            try (var clientCommunicator =
                createClient(helper, null, certificatesPath + "/cacert2.jks")) {
                var obj =
                    ServerPrx.createProxy(
                        clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
                obj.ice_ping();
            }

            // CA1 is not accepted with the initial configuration
            try (var clientCommunicator =
                createClient(helper, null, certificatesPath + "/cacert1.jks")) {
                var obj =
                    ServerPrx.createProxy(
                        clientCommunicator, "server:" + helper.getTestEndpoint(10, "ssl"));
                try {
                    obj.ice_ping();
                    test(false);
                } catch (SecurityException ex) {
                    // Expected
                }
            }

            out.println("ok");
        } catch (Exception ex) {
            System.out.println(ex.toString());
            throw new RuntimeException(ex);
        }
    }

    public static void allTests(TestHelper helper, String testDir) {
        String certificatesPath = testDir + "/../certs";

        clientValidatesServerUsingTrustStore(helper, certificatesPath);
        clientValidatesServerUsingDefaultTrustStore(helper, certificatesPath);
        clientRejectsServerUsingTrustStore(helper, certificatesPath);

        serverValidatesClientUsingTrustStore(helper, certificatesPath);
        serverRejectsClientUsingTrustStore(helper, certificatesPath);

        serverHotCertificateReload(helper, certificatesPath);
    }

    private PlatformTests() {
    }
}
