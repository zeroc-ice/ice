// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceSSL.configuration;
import java.io.PrintWriter;

import test.IceSSL.configuration.Test.ServerFactoryPrx;
import test.IceSSL.configuration.Test.ServerFactoryPrxHelper;
import test.IceSSL.configuration.Test.ServerPrx;

//
// NOTE: This test is not interoperable with other language mappings.
//

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static Ice.InitializationData
    createClientProps(Ice.Properties defaultProperties, String defaultDir, String defaultHost)
    {
        Ice.InitializationData result = new Ice.InitializationData();
        result.properties = Ice.Util.createProperties();
        result.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
        if(defaultProperties.getProperty("Ice.IPv6").length() > 0)
        {
            result.properties.setProperty("Ice.IPv6", defaultProperties.getProperty("Ice.IPv6"));
        }
        result.properties.setProperty("Ice.RetryIntervals", "-1");
        result.properties.setProperty("IceSSL.DefaultDir", defaultDir);
        result.properties.setProperty("IceSSL.Random", "seed.dat");
        if(defaultHost.length() > 0)
        {
            result.properties.setProperty("Ice.Default.Host", defaultHost);
        }
        return result;
    }

    private static java.util.Map<String, String>
    createServerProps(Ice.Properties defaultProperties, String defaultDir, String defaultHost)
    {
        java.util.Map<String, String> result = new java.util.HashMap<String, String>();
        result.put("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
        if(defaultProperties.getProperty("Ice.IPv6").length() > 0)
        {
            result.put("Ice.IPv6", defaultProperties.getProperty("Ice.IPv6"));
        }
        result.put("IceSSL.DefaultDir", defaultDir);
        result.put("IceSSL.Random", "seed.dat");
        if(defaultHost.length() > 0)
        {
            result.put("Ice.Default.Host", defaultHost);
        }
        return result;
    }

    public static ServerFactoryPrx
    allTests(test.Util.Application app, String testDir, PrintWriter out)
    {
        Ice.Communicator communicator = app.communicator();
        final String factoryRef = "factory:tcp -p 12010";
        Ice.ObjectPrx b = communicator.stringToProxy(factoryRef);
        test(b != null);
        ServerFactoryPrx factory = ServerFactoryPrxHelper.checkedCast(b);

        final String defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");
        final String defaultDir = testDir + "/../certs";
        final Ice.Properties defaultProperties = communicator.getProperties();
        final String[] args = new String[0];

        out.print("testing manual initialization... ");
        out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("Ice.InitPlugins", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Ice.ObjectPrx p = comm.stringToProxy("dummy:ssl -p 9999");
            try
            {
                p.ice_ping();
                test(false);
            }
            catch(Ice.PluginInitializationException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("Ice.InitPlugins", "0");
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Ice.PluginManager pm = comm.getPluginManager();
            pm.initializePlugins();
            Ice.ObjectPrx obj = comm.stringToProxy(factoryRef);
            test(obj != null);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(obj);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing certificate verification... ");
        out.flush();
        {
            //
            // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
            // but it still verifies the server's.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "0");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.noCert();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            //
            // Validate that we can get the connection info.
            //
            try
            {
                IceSSL.NativeConnectionInfo info = (IceSSL.NativeConnectionInfo)server.ice_getConnection().getInfo();
                test(info.nativeCerts.length == 2);
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            //
            // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try
            {
                server.noCert();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            //
            // Test IceSSL.VerifyPeer=2. This should fail because the client
            // does not supply a certificate.
            //
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            comm.destroy();

            //
            // Test IceSSL.VerifyPeer=0. This should succeed even though the client
            // does not trust the server's certificate.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "0");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // Test IceSSL.VerifyPeer=1. This should fail because the server
            // does not supply a certificate.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "1");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // Test IceSSL.VerifyPeer=1. Client has a certificate.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try
            {
                char[] password = "password".toCharArray();

                java.io.FileInputStream fis = new java.io.FileInputStream(defaultDir + "/c_rsa_ca1.jks");
                java.security.KeyStore clientKeystore = java.security.KeyStore.getInstance("JKS");
                clientKeystore.load(fis, password);
                java.security.cert.X509Certificate clientCert =
                    (java.security.cert.X509Certificate)clientKeystore.getCertificate("cert");
                server.checkCert(clientCert.getSubjectDN().toString(), clientCert.getIssuerDN().toString());

                fis = new java.io.FileInputStream(defaultDir + "/s_rsa_ca1.jks");
                java.security.KeyStore serverKeystore = java.security.KeyStore.getInstance("JKS");
                serverKeystore.load(fis, password);
                java.security.cert.X509Certificate serverCert =
                    (java.security.cert.X509Certificate)serverKeystore.getCertificate("cert");
                java.security.cert.X509Certificate caCert =
                    (java.security.cert.X509Certificate)serverKeystore.getCertificate("cacert");

                IceSSL.NativeConnectionInfo info = (IceSSL.NativeConnectionInfo)server.ice_getConnection().getInfo();

                test(info.nativeCerts.length == 2);

                test(caCert.equals(info.nativeCerts[1]));
                test(serverCert.equals(info.nativeCerts[0]));
            }
            catch(Exception ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            //
            // Test IceSSL.VerifyPeer=2. Client has a certificate.
            //
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try
            {
                char[] password = "password".toCharArray();

                java.io.FileInputStream fis = new java.io.FileInputStream(defaultDir + "/c_rsa_ca1.jks");
                java.security.KeyStore clientKeystore = java.security.KeyStore.getInstance("JKS");
                clientKeystore.load(fis, password);
                java.security.cert.X509Certificate clientCert =
                    (java.security.cert.X509Certificate)clientKeystore.getCertificate("cert");
                server.checkCert(clientCert.getSubjectDN().toString(), clientCert.getIssuerDN().toString());
            }
            catch(Exception ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            comm.destroy();

            //
            // Test IceSSL.VerifyPeer=1. This should fail because the
            // client doesn't trust the server's CA.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca2.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert2.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            comm.destroy();

            //
            // Test IceSSL.VerifyPeer=2. This should fail because the
            // server doesn't trust the client's CA.
            //
            // NOTE: In C++ this test fails with VerifyPeer=1, but JSSE seems
            // to allow the handshake to continue unless we set VerifyPeer=2.
            //
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca2.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // Verify that IceSSL.CheckCertName has no effect in a server.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);

            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.CheckCertName", "1");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // Test IceSSL.CheckCertName. The test certificates for the server contain "127.0.0.1"
            // as the common name or as a subject alternative name, so we only perform this test when
            // the default host is "127.0.0.1".
            //
            if(defaultHost.equals("127.0.0.1"))
            {
                //
                // Test subject alternative name.
                //
                {
                    initData = createClientProps(defaultProperties, defaultDir, defaultHost);
                    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
                    initData.properties.setProperty("IceSSL.Password", "password");
                    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Ice.Util.initialize(args, initData);

                    fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, defaultDir, defaultHost);
                    d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
                    d.put("IceSSL.Password", "password");
                    d.put("IceSSL.Truststore", "cacert1.jks");
                    server = fact.createServer(d);
                    try
                    {
                        server.ice_ping();
                    }
                    catch(Ice.LocalException ex)
                    {
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }
                //
                // Test common name.
                //
                {
                    initData = createClientProps(defaultProperties, defaultDir, defaultHost);
                    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
                    initData.properties.setProperty("IceSSL.Password", "password");
                    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Ice.Util.initialize(args, initData);

                    fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, defaultDir, defaultHost);
                    d.put("IceSSL.Keystore", "s_rsa_ca1_cn1.jks");
                    d.put("IceSSL.Password", "password");
                    d.put("IceSSL.Truststore", "cacert1.jks");
                    server = fact.createServer(d);
                    try
                    {
                        server.ice_ping();
                    }
                    catch(Ice.LocalException ex)
                    {
                        test(false);
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }
                //
                // Test common name again. The certificate used in this test has "127.0.0.11" as its
                // common name, therefore the address "127.0.0.1" must NOT match.
                //
                {
                    initData = createClientProps(defaultProperties, defaultDir, defaultHost);
                    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
                    initData.properties.setProperty("IceSSL.Password", "password");
                    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
                    initData.properties.setProperty("IceSSL.CheckCertName", "1");
                    comm = Ice.Util.initialize(args, initData);

                    fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
                    test(fact != null);
                    d = createServerProps(defaultProperties, defaultDir, defaultHost);
                    d.put("IceSSL.Keystore", "s_rsa_ca1_cn2.jks");
                    d.put("IceSSL.Password", "password");
                    d.put("IceSSL.Truststore", "cacert1.jks");
                    server = fact.createServer(d);
                    try
                    {
                        server.ice_ping();
                        test(false);
                    }
                    catch(Ice.LocalException ex)
                    {
                        // Expected.
                    }
                    fact.destroyServer(server);
                    comm.destroy();
                }
            }
        }
        out.println("ok");

        out.print("testing custom certificate verifier... ");
        out.flush();
        {
            //
            // ADH is allowed but will not have a certificate.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
            test(plugin != null);
            CertificateVerifierI verifier = new CertificateVerifierI();
            plugin.setCertificateVerifier(verifier);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            ServerPrx server = fact.createServer(d);
            try
            {
                String cipherSub = "DH_anon";
                server.checkCipher(cipherSub);
                IceSSL.NativeConnectionInfo info = (IceSSL.NativeConnectionInfo)server.ice_getConnection().getInfo();
                test(info.cipher.indexOf(cipherSub) >= 0);
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            test(verifier.invoked());
            test(!verifier.hadCert());

            //
            // Have the verifier return false. Close the connection explicitly
            // to force a new connection to be established.
            //
            verifier.reset();
            verifier.returnValue(false);
            server.ice_getConnection().close(false);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            test(verifier.invoked());
            test(!verifier.hadCert());

            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Verify that a server certificate is present.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
            test(plugin != null);
            CertificateVerifierI verifier = new CertificateVerifierI();
            plugin.setCertificateVerifier(verifier);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            test(verifier.invoked());
            test(verifier.hadCert());
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Verify that verifier is installed via property.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.CertVerifier", "test.IceSSL.configuration.CertificateVerifierI");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
            test(plugin != null);
            test(plugin.getCertificateVerifier() != null);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing protocols... ");
        out.flush();
        {
            //
            // This should fail because the client and server have no protocol
            // in common.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.Protocols", "ssl3");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            d.put("IceSSL.Protocols", "tls1");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected for thread pool.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // This should succeed.
            //
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            d.put("IceSSL.Protocols", "tls1, ssl3");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing expired certificates... ");
        out.flush();
        {
            //
            // This should fail because the server's certificate is expired.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1_exp.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // This should fail because the client's certificate is expired.
            //
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1_exp.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected.
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing multiple CA certificates... ");
        out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacerts.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca2.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacerts.jks");
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing passwords... ");
        out.flush();
        {
            //
            // Test password failure.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            // Don't specify the password.
            //initData.properties.setProperty("IceSSL.Password", "password");
            try
            {
                Ice.Util.initialize(args, initData);
                test(false);
            }
            catch(Ice.PluginInitializationException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
        }
        {
            //
            // Test password failure with callback.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("Ice.InitPlugins", "0");
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Ice.PluginManager pm = comm.getPluginManager();
            IceSSL.Plugin plugin = (IceSSL.Plugin)pm.getPlugin("IceSSL");
            test(plugin != null);
            PasswordCallbackI cb = new PasswordCallbackI("bogus");
            plugin.setPasswordCallback(cb);
            try
            {
                pm.initializePlugins();
                test(false);
            }
            catch(Ice.PluginInitializationException ex)
            {
                // Expected.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            comm.destroy();
        }
        {
            //
            // Test installation of password callback.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("Ice.InitPlugins", "0");
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Ice.PluginManager pm = comm.getPluginManager();
            IceSSL.Plugin plugin = (IceSSL.Plugin)pm.getPlugin("IceSSL");
            test(plugin != null);
            PasswordCallbackI cb = new PasswordCallbackI();
            plugin.setPasswordCallback(cb);
            test(plugin.getPasswordCallback() == cb);
            try
            {
                pm.initializePlugins();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            comm.destroy();
        }
        {
            //
            // Test password callback property.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.PasswordCallback", "test.IceSSL.configuration.PasswordCallbackI");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Ice.PluginManager pm = comm.getPluginManager();
            IceSSL.Plugin plugin = (IceSSL.Plugin)pm.getPlugin("IceSSL");
            test(plugin != null);
            test(plugin.getPasswordCallback() != null);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing ciphers... ");
        out.flush();
        {
            //
            // The server has a certificate but the client doesn't. They should
            // negotiate to use ADH since we explicitly enable it.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.Ciphers", "ALL");
            d.put("IceSSL.VerifyPeer", "1");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Configure a server with RSA and DSA certificates.
            //
            // First try a client with a DSA certificate.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // Next try a client with an RSA certificate.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();

            //
            // Next try a client with ADH. This should fail.
            //
            initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            comm = Ice.Util.initialize(args, initData);
            fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected for thread pool.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Configure a server with RSA and a client with DSA. This should fail.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DSS.*)");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected for thread pool.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Configure the server with both RSA and DSA certificates, but use the
            // Alias property to select the RSA certificate. This should fail.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DSS.*)");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Alias", "rsacert");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.SecurityException ex)
            {
                // Expected.
            }
            catch(Ice.ConnectionLostException ex)
            {
                // Expected for thread pool.
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Configure the server with both RSA and DSA certificates, but use the
            // Alias property to select the DSA certificate. This should succeed.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Alias", "dsacert");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                //
                // RSA is used by default, so we examine the negotiated cipher to determine whether
                // DSA was actually used.
                //
                IceSSL.ConnectionInfo info = (IceSSL.ConnectionInfo)server.ice_getConnection().getInfo();
                test(info.cipher.toLowerCase().contains("dss"));
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing IceSSL.TrustOnly... ");
        out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly",
                "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly",
                "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly",
                "C=US, ST=Florida, O=\"ZeroC, Inc.\", OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly",
                  "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly",
                  "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "!CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "!CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "CN=Server");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada,CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "!C=Canada,CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada;CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "!C=Canada;!CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "!CN=Server1"); // Should not match "Server"
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "!CN=Client1"); // Should not match "Client"
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Test rejection when client does not supply a certificate.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.TrustOnly",
                  "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Test rejection when client does not supply a certificate.
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.TrustOnly",
                  "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Rejection takes precedence (client).
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "ST=Florida;!CN=Server;C=US");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            //
            // Rejection takes precedence (server).
            //
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "ST=Florida;!CN=Client;C=US");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing IceSSL.TrustOnly.Client... ");
        out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly.Client",
                "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            // Should have no effect.
            d.put("IceSSL.TrustOnly.Client",
                "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly.Client",
                "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            // Should have no effect.
            d.put("IceSSL.TrustOnly.Client", "CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly.Client", "CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly.Client", "!CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing IceSSL.TrustOnly.Server... ");
        out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            // Should have no effect.
            initData.properties.setProperty("IceSSL.TrustOnly.Server",
                "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server",
                "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server",
                  "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            // Should have no effect.
            initData.properties.setProperty("IceSSL.TrustOnly.Server", "!CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server", "CN=Server");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server", "!CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        out.print("testing IceSSL.TrustOnly.Server.<AdapterName>... ");
        out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server", "CN=bogus");
            d.put("IceSSL.TrustOnly.Server.ServerAdapter",
                "C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server.ServerAdapter",
                  "!C=US, ST=Florida, O=ZeroC\\, Inc., OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);

            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server.ServerAdapter", "CN=bogus");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
                test(false);
            }
            catch(Ice.LocalException ex)
            {
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        {
            Ice.InitializationData initData = createClientProps(defaultProperties, defaultDir, defaultHost);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            ServerFactoryPrx fact = ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map<String, String> d = createServerProps(defaultProperties, defaultDir, defaultHost);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server.ServerAdapter", "!CN=bogus");
            ServerPrx server = fact.createServer(d);
            try
            {
                server.ice_ping();
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        out.println("ok");

        return factory;
    }
}
