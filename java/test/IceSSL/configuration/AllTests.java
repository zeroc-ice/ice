// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

//
// NOTE: This test is not interoperable with other language mappings.
//

class CertificateVerifierI implements IceSSL.CertificateVerifier
{
    CertificateVerifierI()
    {
        reset();
    }

    public boolean
    verify(IceSSL.ConnectionInfo info)
    {
        if(info.certs != null)
        {
            try
            {
                java.util.Collection subjectAltNames =
                    ((java.security.cert.X509Certificate)info.certs[0]).getSubjectAlternativeNames();
                test(subjectAltNames != null);
                java.util.ArrayList ipAddresses = new java.util.ArrayList();
                java.util.ArrayList dnsNames = new java.util.ArrayList();
                java.util.Iterator i = subjectAltNames.iterator();
                while(i.hasNext())
                {
                    java.util.List l = (java.util.List)i.next();
                    test(!l.isEmpty());
                    Integer n = (Integer)l.get(0);
                    if(n.intValue() == 7)
                    {
                        ipAddresses.add((String)l.get(1));
                    }
                    else if(n.intValue() == 2)
                    {
                        dnsNames.add((String)l.get(1));
                    }
                }

                test(dnsNames.contains("server"));
                test(ipAddresses.contains("127.0.0.1"));
            }
            catch(java.security.cert.CertificateParsingException ex)
            {
                test(false);
            }
        }

        _hadCert = info.certs != null;
        _invoked = true;
        return _returnValue;
    }

    void
    reset()
    {
        _returnValue = true;
        _invoked = false;
        _hadCert = false;
    }

    void
    returnValue(boolean b)
    {
        _returnValue = b;
    }

    boolean
    invoked()
    {
        return _invoked;
    }

    boolean
    hadCert()
    {
        return _hadCert;
    }

    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private boolean _returnValue;
    private boolean _invoked;
    private boolean _hadCert;
}

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
    createClientProps(String defaultDir, String defaultHost, boolean threadPool)
    {
        Ice.InitializationData result = new Ice.InitializationData();
        result.properties = Ice.Util.createProperties();
        result.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
        if(!threadPool)
        {
            result.properties.setProperty("Ice.ThreadPerConnection", "1");
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

    private static java.util.Map
    createServerProps(String defaultDir, String defaultHost, boolean threadPool)
    {
        java.util.Map result = new java.util.HashMap();
        result.put("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
        if(!threadPool)
        {
            result.put("Ice.ThreadPerConnection", "1");
        }
        result.put("IceSSL.DefaultDir", defaultDir);
        result.put("IceSSL.Random", "seed.dat");
        if(defaultHost.length() > 0)
        {
            result.put("Ice.Default.Host", defaultHost);
        }
        return result;
    }
    
    public static Test.ServerFactoryPrx
    allTests(Ice.Communicator communicator, String testDir, boolean threadPool)
    {
        final String factoryRef = "factory:tcp -p 12010 -t 10000";
        Ice.ObjectPrx b = communicator.stringToProxy(factoryRef);
        test(b != null);
        Test.ServerFactoryPrx factory = Test.ServerFactoryPrxHelper.checkedCast(b);

        final String defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");
        final String defaultDir = testDir + "/../certs";

        final String[] args = new String[0];

        System.out.print("testing manual initialization... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("Ice.InitPlugins", "0");
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Ice.PluginManager pm = comm.getPluginManager();
            pm.initializePlugins();
            Ice.ObjectPrx obj = comm.stringToProxy(factoryRef);
            test(obj != null);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(obj);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        System.out.print("testing certificate verification... ");
        System.out.flush();
        {
            //
            // Test IceSSL.VerifyPeer=0. Client does not have a certificate,
            // but it still verifies the server's.
            //
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "0");
            Test.ServerPrx server = fact.createServer(d);
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
                IceSSL.ConnectionInfo info = IceSSL.Util.getConnectionInfo(server.ice_getConnection());
                test(info.certs.length == 2);
            }
            catch(IceSSL.ConnectionInvalidException ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            //
            // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
            //
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            // Test IceSSL.VerifyPeer=1. This should fail because the server
            // does not supply a certificate.
            //
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "1");
            comm = Ice.Util.initialize(args, initData);
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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

                IceSSL.ConnectionInfo info = IceSSL.Util.getConnectionInfo(server.ice_getConnection());

                test(info.certs.length == 2);

                test(caCert.equals(info.certs[1]));
                test(serverCert.equals(info.certs[0]));
            }
            catch(Exception ex)
            {
                test(false);
            }
            fact.destroyServer(server);

            //
            // Test IceSSL.VerifyPeer=2. Client has a certificate.
            //
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca2.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert2.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);

            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
        }
        System.out.println("ok");

        System.out.print("testing custom certificate verifier... ");
        System.out.flush();
        {
            //
            // ADH is allowed but will not have a certificate.
            //
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
            test(plugin != null);
            CertificateVerifierI verifier = new CertificateVerifierI();
            plugin.setCertificateVerifier(verifier);

            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            d.put("IceSSL.VerifyPeer", "0");
            Test.ServerPrx server = fact.createServer(d);
            try
            {
                String cipherSub = "DH_anon";
                server.checkCipher(cipherSub);
                IceSSL.ConnectionInfo info = IceSSL.Util.getConnectionInfo(server.ice_getConnection());
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
            test(plugin != null);
            CertificateVerifierI verifier = new CertificateVerifierI();
            plugin.setCertificateVerifier(verifier);

            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        System.out.print("testing protocols... ");
        System.out.flush();
        {
            //
            // This should fail because the client and server have no protocol
            // in common.
            //
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.Protocols", "ssl3");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            d.put("IceSSL.Protocols", "tls1");
            Test.ServerPrx server = fact.createServer(d);
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
                test(threadPool);
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
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
        System.out.println("ok");

        System.out.print("testing expired certificates... ");
        System.out.flush();
        {
            //
            // This should fail because the server's certificate is expired.
            //
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca1_exp.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            Test.ServerPrx server = fact.createServer(d);
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
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
        System.out.println("ok");

        System.out.print("testing multiple CA certificates... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacerts.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca2.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacerts.jks");
            d.put("IceSSL.VerifyPeer", "2");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        System.out.print("testing password failure... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
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
        System.out.println("ok");

        System.out.print("testing ciphers... ");
        System.out.flush();
        {
            //
            // The server has a certificate but the client doesn't. They should
            // negotiate to use ADH since we explicitly enable it.
            //
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            initData.properties.setProperty("IceSSL.VerifyPeer", "0");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.Ciphers", "ALL");
            d.put("IceSSL.VerifyPeer", "1");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "1");
            Test.ServerPrx server = fact.createServer(d);
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
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            comm = Ice.Util.initialize(args, initData);
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
            comm = Ice.Util.initialize(args, initData);
            fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            d = createServerProps(defaultDir, defaultHost, threadPool);
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
                test(threadPool);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DSS.*)");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            Test.ServerPrx server = fact.createServer(d);
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
                test(threadPool);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DSS.*)");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Alias", "rsacert");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.VerifyPeer", "2");
            Test.ServerPrx server = fact.createServer(d);
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
                test(threadPool);
            }
            catch(Ice.LocalException ex)
            {
                test(false);
            }
            fact.destroyServer(server);
            comm.destroy();
        }
        System.out.println("ok");

        System.out.print("testing IceSSL.TrustOnly... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly",
                "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly",
                  "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "CN=Client");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly", "CN=Server");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada,CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly", "C=Canada;CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        System.out.print("testing IceSSL.TrustOnly.Client... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly.Client",
                "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            // Should have no effect.
            d.put("IceSSL.TrustOnly.Client",
                "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Server");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            initData.properties.setProperty("IceSSL.TrustOnly.Client", "CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        System.out.print("testing IceSSL.TrustOnly.Server... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            // Should have no effect.
            initData.properties.setProperty("IceSSL.TrustOnly.Server",
                "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server",
                "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server", "CN=Server");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        System.out.print("testing IceSSL.TrustOnly.Server.<AdapterName>... ");
        System.out.flush();
        {
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server", "CN=bogus");
            d.put("IceSSL.TrustOnly.Server.ServerAdapter",
                "C=US, ST=Florida, O=ZeroC\\, Inc.,OU=Ice, emailAddress=info@zeroc.com, CN=Client");
            Test.ServerPrx server = fact.createServer(d);
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
            Ice.InitializationData initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData = createClientProps(defaultDir, defaultHost, threadPool);
            initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
            initData.properties.setProperty("IceSSL.Password", "password");
            initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
            Ice.Communicator comm = Ice.Util.initialize(args, initData);
            Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
            test(fact != null);
            java.util.Map d = createServerProps(defaultDir, defaultHost, threadPool);
            d.put("IceSSL.Keystore", "s_rsa_dsa_ca1.jks");
            d.put("IceSSL.Password", "password");
            d.put("IceSSL.Truststore", "cacert1.jks");
            d.put("IceSSL.TrustOnly.Server.ServerAdapter", "CN=bogus");
            Test.ServerPrx server = fact.createServer(d);
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
        System.out.println("ok");

        return factory;
    }
}
