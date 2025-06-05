// Copyright (c) ZeroC, Inc.

package test.IceSSL.configuration;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.ConnectTimeoutException;
import com.zeroc.Ice.ConnectionLostException;
import com.zeroc.Ice.DNSException;
import com.zeroc.Ice.InitializationData;
import com.zeroc.Ice.InitializationException;
import com.zeroc.Ice.LocalException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.Properties;
import com.zeroc.Ice.SSL.ConnectionInfo;
import com.zeroc.Ice.SecurityException;
import com.zeroc.Ice.SocketException;
import com.zeroc.Ice.Util;
import com.zeroc.Ice.WSConnectionInfo;

import test.IceSSL.configuration.Test.ServerFactoryPrx;
import test.IceSSL.configuration.Test.ServerPrx;
import test.TestHelper;

import java.io.FileInputStream;
import java.io.PrintWriter;
import java.security.KeyStore;
import java.security.cert.X509Certificate;
import java.util.HashMap;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static X509Certificate loadCertificate(String path, String alias) {
        try {
            KeyStore keystore = KeyStore.getInstance("JKS");
            keystore.load(new FileInputStream(path), "password".toCharArray());
            return (X509Certificate) keystore.getCertificate(alias);
        } catch (Exception ex) {
            test(false);
            return null;
        }
    }

    private static InitializationData createClientProps(
            Properties defaultProperties) {
        InitializationData initData = new InitializationData();
        initData.properties = new Properties();
        initData.properties.setProperty("IceSSL.DefaultDir", defaultProperties.getIceProperty("IceSSL.DefaultDir"));
        initData.properties.setProperty("Ice.Default.Host", defaultProperties.getIceProperty("Ice.Default.Host"));
        if (defaultProperties.getIceProperty("Ice.IPv6").length() > 0) {
            initData.properties.setProperty("Ice.IPv6", defaultProperties.getIceProperty("Ice.IPv6"));
        }
        initData.properties.setProperty("Ice.RetryIntervals", "-1");
        return initData;
    }

    private static InitializationData createClientProps(
        Properties defaultProperties, String ks, String ts, String keystoreType) {
        String keyStoreExt = "PKCS12".equals(keystoreType) ? ".p12" : ".jks";
        InitializationData initData = createClientProps(defaultProperties);
        if (!ks.isEmpty()) {
            initData.properties.setProperty("IceSSL.Keystore", ks + keyStoreExt);
            initData.properties.setProperty("IceSSL.KeystoreType", keystoreType);
            initData.properties.setProperty("IceSSL.KeystorePassword", "password");
        }
        if (!ts.isEmpty()) {
            initData.properties.setProperty("IceSSL.Truststore", ts + keyStoreExt);
            initData.properties.setProperty("IceSSL.TruststoreType", keystoreType);
            initData.properties.setProperty("IceSSL.TruststorePassword", "password");
        }
        initData.properties.setProperty("IceSSL.Password", "password");
        return initData;
    }

    private static Map<String, String> createServerProps(Properties defaultProperties) {
        Map<String, String> result = new HashMap<>();
        result.put("IceSSL.DefaultDir", defaultProperties.getIceProperty("IceSSL.DefaultDir"));
        result.put("Ice.Default.Host", defaultProperties.getIceProperty("Ice.Default.Host"));
        result.put("ServerAdapter.PublishedHost", result.get("Ice.Default.Host"));

        if (defaultProperties.getIceProperty("Ice.IPv6").length() > 0) {
            result.put("Ice.IPv6", defaultProperties.getIceProperty("Ice.IPv6"));
        }
        return result;
    }

    private static Map<String, String> createServerProps(
        Properties defaultProperties, String ks, String ts, String keystoreType) {

        String keyStoreExt = "PKCS12".equals(keystoreType) ? ".p12" : ".jks";
        Map<String, String> d = createServerProps(defaultProperties);
        if (!ks.isEmpty()) {
            d.put("IceSSL.Keystore", ks + keyStoreExt);
            d.put("IceSSL.KeystoreType", keystoreType);
            d.put("IceSSL.KeystorePassword", "password");
        }
        if (!ts.isEmpty()) {
            d.put("IceSSL.Truststore", ts + keyStoreExt);
            d.put("IceSSL.TruststoreType", keystoreType);
            d.put("IceSSL.TruststorePassword", "password");
        }
        d.put("IceSSL.Password", "password");
        return d;
    }

    public static ServerFactoryPrx allTests(TestHelper helper, String defaultDir, String keystoreType) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();

        out.println("Running tests with " + keystoreType + " certificates");

        final String factoryRef = "factory:" + helper.getTestEndpoint(0, "tcp");
        ObjectPrx b = communicator.stringToProxy(factoryRef);
        test(b != null);
        final ServerFactoryPrx factory = ServerFactoryPrx.checkedCast(b);

        final String defaultHost = communicator.getProperties().getIceProperty("Ice.Default.Host");
        final Properties defaultProperties = communicator.getProperties()._clone();
        final String[] args = new String[0];
        defaultProperties.setProperty("IceSSL.DefaultDir", defaultDir);
        defaultProperties.setProperty("Ice.Default.Host", defaultHost);

        out.print("testing certificate verification... ");
        out.flush();
        {
            Map<String, String> d;
            InitializationData initData;

            // Test Ice.SSL.VerifyPeer=0. Client does not have a certificate, but it still verifies the server's.
            initData = createClientProps(defaultProperties, "", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "", keystoreType);
            d.put("IceSSL.VerifyPeer", "0");
            ServerPrx server = fact.createServer(d);
            try {
                server.noCert();
                test(((ConnectionInfo) server.ice_getConnection().getInfo()).verified);
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            // Test Ice.SSL.VerifyPeer=1. Client does not have a certificate.
            initData = createClientProps(defaultProperties, "", "ca1/ca1", keystoreType);
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "", keystoreType);
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try {
                server.noCert();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);

            // Test Ice.SSL.VerifyPeer=2. This should fail because the client does not supply a certificate.
            d = createServerProps(defaultProperties, "ca1/server", "", keystoreType);
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (SecurityException ex) {
                // Expected.
            } catch (ConnectionLostException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);

            comm.destroy();

            // Test Ice.SSL.VerifyPeer=1. Client has a certificate.
            //
            // Provide "ca1/ca1" to the client to verify the server certificate (without this the client connection
            // wouldn't be able to provide the certificate chain).
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try {
                X509Certificate clientCert = loadCertificate(defaultDir + "/ca1/client.jks", "client");
                server.checkCert(
                    clientCert.getSubjectX500Principal().toString(),
                    clientCert.getIssuerX500Principal().toString());

                X509Certificate serverCert = loadCertificate(defaultDir + "/ca1/server.jks", "server");

                ConnectionInfo info = (ConnectionInfo) server.ice_getConnection().getInfo();

                test(info.certs.length == 2);
                test(info.verified);

                test(serverCert.equals(info.certs[0]));
            } catch (Exception ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);

            // Test Ice.SSL.VerifyPeer=2. Client has a certificate.
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try {
                X509Certificate clientCert = loadCertificate(defaultDir + "/ca1/client.jks", "client");
                server.checkCert(
                    clientCert.getSubjectX500Principal().toString(),
                    clientCert.getIssuerX500Principal().toString());
            } catch (Exception ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);

            comm.destroy();

            // Test Ice.SSL.VerifyPeer=1. This should fail because the client doesn't trust the server's CA.
            initData = createClientProps(defaultProperties, "", "", keystoreType);
            initData.properties.setProperty("IceSSL.VerifyPeer", "1");
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.VerifyPeer", "0");
            server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (SecurityException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);

            comm.destroy();

            // Test Ice.SSL.VerifyPeer=1. This should fail because the server doesn't trust the client's CA.
            initData = createClientProps(defaultProperties, "ca2/client", "", keystoreType);
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "", keystoreType);
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (SecurityException ex) {
                // Expected.
            } catch (ConnectionLostException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            // This should succeed because the self signed certificate used by the server is trusted.
            initData = createClientProps(defaultProperties, "", "ca2/ca2", keystoreType);
            initData.properties.setProperty("IceSSL.VerifyPeer", "1");
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca2/ca2", "", keystoreType);
            d.put("IceSSL.VerifyPeer", "0");
            server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            // This should fail because the self signed certificate used by the server is not trusted.
            initData = createClientProps(defaultProperties);
            initData.properties.setProperty("IceSSL.VerifyPeer", "1");
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca2/ca2", "", keystoreType);
            d.put("IceSSL.VerifyPeer", "0");
            server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (SecurityException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            // Verify that Ice.SSL.CheckCertName has no effect in a server.
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            comm = Util.initialize(args, initData);

            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.CheckCertName", "1");
            server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            // Test Hostname verification only when Ice.DefaultHost is 127.0.0.1 as that is the IP address used in the
            // test certificates.
            if ("127.0.0.1".equals(defaultHost)) {
                // Test using localhost as target host
                Properties props = defaultProperties._clone();
                props.setProperty("Ice.Default.Host", "localhost");
                // Target host matches the certificate DNS altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(props, "ca1/server_cn1", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                    } catch (LocalException ex) {
                        ex.printStackTrace();
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host does not match the certificate DNS altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(props, "ca1/server_cn2", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host matches the certificate Common Name and the certificate does not include a DNS altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(props, "ca1/server_cn3", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                    } catch (LocalException ex) {
                        ex.printStackTrace();
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host does not match the certificate Common Name and the certificate does not include a DNS
                // altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(props, "ca1/server_cn4", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host matches the certificate Common Name and the certificate has a DNS altName that does not
                // match the target host
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(props, "ca1/server_cn5", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Test using 127.0.0.1 as target host

                // Target host matches the certificate IP altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, "ca1/server_cn6", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                    } catch (LocalException ex) {
                        ex.printStackTrace();
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host does not match the certificate IP altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, "ca1/server_cn7", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host is an IP address that matches the CN and the certificate doesn't include an IP altName
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, "ca1/server_cn8", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                        test(false);
                    } catch (SecurityException ex) {
                        // Expected
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }

                // Target host does not match the certificate DNS altName, connection should succeed because
                // Ice.SSL.CheckCertName is set to 0.
                {
                    initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
                    initData.properties.setProperty("IceSSL.CheckCertName", "0");
                    comm = Util.initialize(args, initData);

                    fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(props, "ca1/server_cn2", "ca1/ca1", keystoreType);
                    server = fact.createServer(d);
                    try {
                        server.ice_ping();
                    } catch (LocalException ex) {
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }
            }
        }
        out.println("ok");

        InitializationData initData;
        Map<String, String> d;

        out.print("testing expired certificates... ");
        out.flush();
        {
            // This should fail because the server's certificate is expired.
            initData = createClientProps(defaultProperties, "ca1/client", "ca5/ca5", keystoreType);
            Communicator comm = Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca5/server_expired", "ca1/ca1", keystoreType);
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (SecurityException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            // This should fail because the client's certificate is expired.
            initData = createClientProps(defaultProperties, "ca5/client_expired", "ca1/ca1", keystoreType);
            comm = Util.initialize(args, initData);
            fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca5/ca5", keystoreType);
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (ConnectionLostException ex) {
                // Expected.
            } catch (SecurityException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing multiple CA certificates... ");
        out.flush();
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca_all", keystoreType);
            Communicator comm = Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca2/server", "ca_all", keystoreType);
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing passwords... ");
        out.flush();
        {
            // Test password failure.
            initData = createClientProps(defaultProperties);
            initData.properties.setProperty(
                "IceSSL.Keystore",
                "ca1/ca1." + ("PKCS12".equals(keystoreType) ? "p12" : "jks"));
            initData.properties.setProperty("IceSSL.KeystoreType", keystoreType);
            // Don't specify the password.
            try {
                Util.initialize(args, initData);
                test(false);
            } catch (InitializationException ex) {
                // Expected.
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
        }
        out.println("ok");

        out.print("testing Ice.SSL.TrustOnly... ");
        out.flush();
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty(
                "IceSSL.TrustOnly",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty(
                "IceSSL.TrustOnly",
                "!C=US, ST=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty(
                "IceSSL.TrustOnly",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC, OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put(
                "IceSSL.TrustOnly",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put(
                "IceSSL.TrustOnly",
                "!C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "!CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly", "CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly", "!CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "CN=ca1.client");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly", "CN=ca1.server");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada,CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "!C=Canada,CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada;CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "!C=Canada;!CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty(
                "IceSSL.TrustOnly", "!CN=ca1.server1"); // Should not match "Server"
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly", "!CN=ca1.client_x"); // Should not match "ca1.client"
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            // Rejection takes precedence (client).
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly", "ST=Florida;!CN=ca1.server;C=US");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            // Rejection takes precedence (server).
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly", "ST=Florida;!CN=ca1.client;C=US");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing Ice.SSL.TrustOnly.Client... ");
        out.flush();
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty(
                "IceSSL.TrustOnly.Client",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            // Should have no effect.
            d.put(
                "IceSSL.TrustOnly.Client",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.server");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty(
                "IceSSL.TrustOnly.Client",
                "!C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            // Should have no effect.
            d.put("IceSSL.TrustOnly.Client", "CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly.Client", "CN=ca1.client");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            initData.properties.setProperty("IceSSL.TrustOnly.Client", "!CN=ca1.client");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing Ice.SSL.TrustOnly.Server... ");
        out.flush();
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            // Should have no effect.
            initData.properties.setProperty(
                "IceSSL.TrustOnly.Server",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put(
                "IceSSL.TrustOnly.Server",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put(
                "IceSSL.TrustOnly.Server",
                "!C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            // Should have no effect.
            initData.properties.setProperty("IceSSL.TrustOnly.Server", "!CN=ca1.server");
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly.Server", "CN=ca1.server");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly.Server", "!CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing Ice.SSL.TrustOnly.Server.<AdapterName>... ");
        out.flush();
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly.Server", "CN=bogus");
            d.put(
                "IceSSL.TrustOnly.Server.ServerAdapter",
                "C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put(
                "IceSSL.TrustOnly.Server.ServerAdapter",
                "!C=US, ST=Florida, L=Jupiter, O=ZeroC,OU=Ice test infrastructure,"
                    + " emailAddress=info@zeroc.com, CN=ca1.client");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly.Server.ServerAdapter", "CN=bogus");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
                test(false);
            } catch (LocalException ex) {}
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            initData = createClientProps(defaultProperties, "ca1/client", "ca1/ca1", keystoreType);
            Communicator comm = Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrx.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, "ca1/server", "ca1/ca1", keystoreType);
            d.put("IceSSL.TrustOnly.Server.ServerAdapter", "!CN=bogus");
            ServerPrx server = fact.createServer(d);
            try {
                server.ice_ping();
            } catch (LocalException ex) {
                ex.printStackTrace();
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing system CAs... ");
        out.flush();
        {
            // Retry a few times in case there are connectivity problems with demo.zeroc.com.
            final int retryMax = 5;
            final int retryDelay = 1000;
            int retryCount = 0;

            initData = createClientProps(defaultProperties);
            Communicator comm = Util.initialize(initData);
            ObjectPrx p =
                comm.stringToProxy(
                    "dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
            while (true) {
                try {
                    p.ice_ping();
                    test(false);
                } catch (SecurityException ex) {
                    // Expected, by default we don't check for system CAs.
                    break;
                } catch (LocalException ex) {
                    if ((ex instanceof ConnectTimeoutException)
                        || (ex instanceof SocketException)
                        || (ex instanceof DNSException)) {
                        if (++retryCount < retryMax) {
                            out.print("retrying... ");
                            out.flush();
                            try {
                                Thread.sleep(retryDelay);
                            } catch (InterruptedException e) {
                                break;
                            }
                            continue;
                        }
                    }

                    out.print("warning: unable to connect to demo.zeroc.com to check system CA\n");
                    ex.printStackTrace();
                    break;
                }
            }
            comm.destroy();

            retryCount = 0;
            initData = createClientProps(defaultProperties);
            initData.properties.setProperty("IceSSL.UsePlatformCAs", "1");
            comm = Util.initialize(initData);
            p = comm.stringToProxy("dummy:wss -p 443 -h zeroc.com -r /demo-proxy/chat/glacier2");
            while (true) {
                try {
                    WSConnectionInfo info =
                        (WSConnectionInfo) p.ice_getConnection().getInfo();
                    ConnectionInfo sslinfo =
                        (ConnectionInfo) info.underlying;
                    test(sslinfo.verified);
                    break;
                } catch (LocalException ex) {
                    if ((ex instanceof ConnectTimeoutException)
                        || (ex instanceof SocketException)
                        || (ex instanceof DNSException)) {
                        if (++retryCount < retryMax) {
                            out.print("retrying... ");
                            out.flush();
                            try {
                                Thread.sleep(retryDelay);
                            } catch (InterruptedException e) {
                                break;
                            }
                            continue;
                        }
                    }

                    out.print("warning: unable to connect to demo.zeroc.com to check system CA\n");
                    ex.printStackTrace();
                    break;
                }
            }
            comm.destroy();
        }
        out.println("ok");

        return factory;
    }

    private AllTests() {}
}
