// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    verify(IceSSL.VerifyInfo info)
    {
	_incoming = info.incoming;
	_hadCert = info.certs != null;
	_invoked = true;
	return _returnValue;
    }

    void
    reset()
    {
	_returnValue = true;
       	_invoked = false;
	_incoming = false;
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
    incoming()
    {
	return _incoming;
    }

    boolean
    hadCert()
    {
	return _hadCert;
    }

    private boolean _returnValue;
    private boolean _invoked;
    private boolean _incoming;
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
    createClientProps(String testDir, String defaultHost)
    {
	Ice.InitializationData result = new Ice.InitializationData();
	result.properties = Ice.Util.createProperties();
	result.properties.setProperty("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
	result.properties.setProperty("Ice.ThreadPerConnection", "1");
	result.properties.setProperty("Ice.RetryIntervals", "-1");
	if(defaultHost.length() > 0)
	{
	    result.properties.setProperty("Ice.Default.Host", defaultHost);
	}
	return result;
    }

    private static java.util.Map
    createServerProps(String testDir, String defaultHost)
    {
	java.util.Map result = new java.util.HashMap();
	result.put("Ice.Plugin.IceSSL", "IceSSL.PluginFactory");
	result.put("Ice.ThreadPerConnection", "1");
	if(defaultHost.length() > 0)
	{
	    result.put("Ice.Default.Host", defaultHost);
	}
	return result;
    }
    
    public static void
    allTests(Ice.Communicator communicator, String testDir)
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("Ice.InitPlugins", "0");
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
	    initData.properties.setProperty("IceSSL.VerifyPeer", "0");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Ice.PluginManager pm = comm.getPluginManager();
	    pm.initializePlugins();
	    Ice.ObjectPrx obj = comm.stringToProxy(factoryRef);
	    test(obj != null);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(obj);
	    java.util.Map d = createServerProps(testDir, defaultHost);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
	    d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
	    d.put("IceSSL.Password", "password");
	    d.put("IceSSL.Truststore", "cacert1.jks");
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

	    //
	    // Test IceSSL.VerifyPeer=1. Client does not have a certificate.
	    //
	    initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
	    d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
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

	    //
	    // Test IceSSL.VerifyPeer=2. This should fail because the client
	    // does not supply a certificate.
	    //
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
	    initData.properties.setProperty("IceSSL.VerifyPeer", "1");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
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
	    initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
	    d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
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

	    //
	    // Test IceSSL.VerifyPeer=2. Client has a certificate.
	    //
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
	    d.put("IceSSL.Keystore", "s_rsa_ca1.jks");
	    d.put("IceSSL.Password", "password");
	    d.put("IceSSL.Truststore", "cacert1.jks");
	    d.put("IceSSL.VerifyPeer", "2");
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
	    // Test IceSSL.VerifyPeer=1. This should fail because the
	    // client doesn't trust the server's CA.
	    //
	    initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca2.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert2.jks");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca2.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	}
	System.out.println("ok");

	System.out.print("testing custom certificate verifier... ");
	System.out.flush();
	{
	    //
	    // ADH is allowed but will not have a certificate.
	    //
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
	    initData.properties.setProperty("IceSSL.VerifyPeer", "0");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
	    test(plugin != null);
	    CertificateVerifierI verifier = new CertificateVerifierI();
	    plugin.setCertificateVerifier(verifier);

	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
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
	    test(verifier.invoked());
	    test(!verifier.incoming());
	    test(!verifier.hadCert());

	    //
	    // Have the verifier return false. Close the connection explicitly
	    // to force a new connection to be established.
	    //
	    verifier.reset();
	    verifier.returnValue(false);
	    server.ice_connection().close(false);
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
	    test(!verifier.incoming());
	    test(!verifier.hadCert());

	    fact.destroyServer(server);
	    comm.destroy();
	}
	{
	    //
	    // Verify that a server certificate is present.
	    //
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
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
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    test(!verifier.incoming());
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    initData.properties.setProperty("IceSSL.Protocols", "ssl3");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1_exp.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacerts.jks");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
	    initData.properties.setProperty("IceSSL.VerifyPeer", "0");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_rsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DH_anon.*)");
	    comm = Ice.Util.initialize(args, initData);
	    fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DSS.*)");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    Ice.InitializationData initData = createClientProps(testDir, defaultHost);
	    initData.properties.setProperty("IceSSL.DefaultDir", defaultDir);
	    initData.properties.setProperty("IceSSL.Keystore", "c_dsa_ca1.jks");
	    initData.properties.setProperty("IceSSL.Password", "password");
	    initData.properties.setProperty("IceSSL.Truststore", "cacert1.jks");
	    initData.properties.setProperty("IceSSL.Ciphers", "NONE (.*DSS.*)");
	    Ice.Communicator comm = Ice.Util.initialize(args, initData);
	    Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
	    test(fact != null);
	    java.util.Map d = createServerProps(testDir, defaultHost);
	    d.put("IceSSL.DefaultDir", defaultDir);
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
	    catch(Ice.LocalException ex)
	    {
		test(false);
	    }
	    fact.destroyServer(server);
	    comm.destroy();
	}
	System.out.println("ok");

	factory.shutdown();
    }
}
