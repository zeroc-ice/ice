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

using System;
using System.Security.Cryptography.X509Certificates;

internal class CertificateVerifierI : IceSSL.CertificateVerifier
{
    internal CertificateVerifierI()
    {
	reset();
    }

    public bool verify(IceSSL.VerifyInfo info)
    {
	incoming_ = info.incoming;
	hadCert_ = info.cert != null;
	invoked_ = true;
	return returnValue_;
    }

    internal void reset()
    {
	returnValue_ = true;
       	invoked_ = false;
	incoming_ = false;
	hadCert_ = false;
    }

    internal void returnValue(bool b)
    {
	returnValue_ = b;
    }

    internal bool invoked()
    {
	return invoked_;
    }

    internal bool incoming()
    {
	return incoming_;
    }

    internal bool hadCert()
    {
	return hadCert_;
    }

    private bool returnValue_;
    private bool invoked_;
    private bool incoming_;
    private bool hadCert_;
};

public class AllTests
{
    private static void test(bool b)
    {
	if (!b)
	{
	    throw new Exception();
	}
    }

    private static Ice.Properties
    createClientProps(string testDir, string defaultHost)
    {
	Ice.Properties result = Ice.Util.createProperties();
	//
	// TODO:
	//
	// When an application loads icesslcs.dll directly, as this one does, we
	// must ensure that it uses the same DLL as the one loaded dynamically
	// by Ice. Since Visual Studio copies icesslcs.dll into the test
	// directory, we'll use that one.
	//
	// When Mono supports .NET 2.0, we'll need to fix this.
	//
	result.setProperty("Ice.Plugin.IceSSL", testDir + "/icesslcs.dll:IceSSL.PluginFactory");
	result.setProperty("Ice.ThreadPerConnection", "1");
	if(defaultHost.Length > 0)
	{
	    result.setProperty("Ice.Default.Host", defaultHost);
	}
	return result;
    }

    private static Test.Properties
    createServerProps(string testDir, string defaultHost)
    {
	Test.Properties result = new Test.Properties();
	result["Ice.Plugin.IceSSL"] = testDir + "/../../../bin/icesslcs.dll:IceSSL.PluginFactory";
	result["Ice.ThreadPerConnection"] = "1";
	if(defaultHost.Length > 0)
	{
	    result["Ice.Default.Host"] = defaultHost;
	}
	return result;
    }
    
    public static void allTests(Ice.Communicator communicator, string testDir)
    {
	string factoryRef = "factory:tcp -p 12010 -t 10000";
	Ice.ObjectPrx b = communicator.stringToProxy(factoryRef);
	test(b != null);
	Test.ServerFactoryPrx factory = Test.ServerFactoryPrxHelper.checkedCast(b);

	string defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");
	string defaultDir = testDir + "/../certs";

	//
	// Load the CA certificates. We could use the IceSSL.ImportCert property, but
	// it would be nice to remove the CA certificates when the test finishes, so
	// this test manually installs the certificates in the LocalMachine:AuthRoot
	// store.
	//
	// Note that the client and server are assumed to run on the same machine,
	// so the certificates installed by the client are also available to the
	// server.
	//
	string caCert1File = defaultDir + "/cacert1.pem";
	string caCert2File = defaultDir + "/cacert2.pem";
	X509Certificate2 caCert1 = new X509Certificate2(caCert1File);
	X509Certificate2 caCert2 = new X509Certificate2(caCert2File);
	X509Store store = new X509Store(StoreName.AuthRoot, StoreLocation.LocalMachine);
	store.Open(OpenFlags.ReadWrite);

	try
	{
	    string[] args = new string[0];

	    Console.Out.Write("testing manual initialization... ");
	    Console.Out.Flush();
	    {
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.DelayInit", "1");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		Ice.ObjectPrx p = comm.stringToProxy("dummy:ssl -p 9999");
		try
		{
		    p.ice_ping();
		    test(false);
		}
		catch(Ice.PluginInitializationException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		comm.destroy();
	    }
	    {
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.DelayInit", "1");
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
		test(plugin != null);
		plugin.initialize(null);
		Ice.ObjectPrx obj = comm.stringToProxy(factoryRef);
		test(obj != null);
		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(obj);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		store.Add(caCert1);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		comm.destroy();
	    }
	    {
		//
		// Supply our own certificate.
		//
		X509Certificate2 cert = new X509Certificate2(defaultDir + "/c_rsa_nopass_ca1.pfx", "password");
		X509Certificate2Collection coll = new X509Certificate2Collection();
		coll.Add(cert);
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.DelayInit", "1");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
		test(plugin != null);
		plugin.initialize(coll);
		Ice.ObjectPrx obj = comm.stringToProxy(factoryRef);
		test(obj != null);
		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(obj);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		comm.destroy();
	    }
	    Console.Out.WriteLine("ok");

	    Console.Out.Write("testing certificate verification... ");
	    Console.Out.Flush();
	    {
		//
		// Test IceSSL.VerifyPeer=1. Client does not have a certificate.
		//
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "1";
		store.Add(caCert1);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);

		//
		// Test IceSSL.VerifyPeer=2. This should fail because the client
		// does not supply a certificate.
		//
		d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		    test(false);
		}
		catch(Ice.ConnectionLostException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);

		comm.destroy();

		//
		// Test IceSSL.VerifyPeer=1. Client has a certificate.
		//
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		comm = Ice.Util.initialize(ref args, initData);
		fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "1";
		store.Add(caCert1);
		server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);

		//
		// Test IceSSL.VerifyPeer=2. Client has a certificate.
		//
		d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);

		comm.destroy();

		//
		// Test IceSSL.VerifyPeer=1. This should fail because the
		// client doesn't trust the server's CA.
		//
		initData.properties = createClientProps(testDir, defaultHost);
		comm = Ice.Util.initialize(ref args, initData);
		fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "1";
		// Don't add the CA certificate.
		//store.Add(caCert1);
		server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		    test(false);
		}
		catch(Ice.SecurityException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		comm.destroy();
	    }
	    Console.Out.WriteLine("ok");

	    Console.Out.Write("testing custom certificate verifier... ");
	    Console.Out.Flush();
	    {
		//
		// Verify that a server certificate is present.
		//
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		IceSSL.Plugin plugin = (IceSSL.Plugin)comm.getPluginManager().getPlugin("IceSSL");
		test(plugin != null);
		CertificateVerifierI verifier = new CertificateVerifierI();
		plugin.setCertificateVerifier(verifier);

		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		test(verifier.invoked());
		test(!verifier.incoming());
		test(verifier.hadCert());

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
		catch(Ice.SecurityException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		test(verifier.invoked());
		test(!verifier.incoming());
		test(verifier.hadCert());
		fact.destroyServer(server);
		store.Remove(caCert1);

		comm.destroy();
	    }
	    Console.Out.WriteLine("ok");

	    Console.Out.Write("testing protocols... ");
	    Console.Out.Flush();
	    {
		//
		// This should fail because the client and server have no protocol
		// in common.
		//
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		initData.properties.setProperty("IceSSL.Protocols", "ssl3");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		d["IceSSL.Protocols"] = "tls1";
		store.Add(caCert1);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		    test(false);
		}
		catch(Ice.ConnectionLostException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		comm.destroy();

		//
		// This should succeed.
		//
		comm = Ice.Util.initialize(ref args, initData);
		fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		d["IceSSL.Protocols"] = "tls1, ssl3";
		store.Add(caCert1);
		server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		comm.destroy();
	    }
	    Console.Out.WriteLine("ok");

	    Console.Out.Write("testing expired certificates... ");
	    Console.Out.Flush();
	    {
		//
		// This should fail because the server's certificate is expired.
		//
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1_exp.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		    test(false);
		}
		catch(Ice.SecurityException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		comm.destroy();

		//
		// This should fail because the client's certificate is expired.
		//
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1_exp.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		comm = Ice.Util.initialize(ref args, initData);
		fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca1.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		    test(false);
		}
		catch(Ice.ConnectionLostException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		comm.destroy();
	    }
	    Console.Out.WriteLine("ok");

	    Console.Out.Write("testing multiple CA certificates... ");
	    Console.Out.Flush();
	    {
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		initData.properties.setProperty("IceSSL.Password", "password");
		Ice.Communicator comm = Ice.Util.initialize(ref args, initData);
		Test.ServerFactoryPrx fact = Test.ServerFactoryPrxHelper.checkedCast(comm.stringToProxy(factoryRef));
		test(fact != null);
		Test.Properties d = createServerProps(testDir, defaultHost);
		d["IceSSL.CertFile"] = defaultDir + "/s_rsa_nopass_ca2.pfx";
		d["IceSSL.Password"] = "password";
		d["IceSSL.VerifyPeer"] = "2";
		store.Add(caCert1);
		store.Add(caCert2);
		Test.ServerPrx server = fact.createServer(d);
		try
		{
		    server.ice_ping();
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
		fact.destroyServer(server);
		store.Remove(caCert1);
		store.Remove(caCert2);
		comm.destroy();
	    }
	    Console.Out.WriteLine("ok");

	    Console.Out.Write("testing password failure... ");
	    Console.Out.Flush();
	    {
	    	Ice.InitializationData initData = new Ice.InitializationData();
		initData.properties = createClientProps(testDir, defaultHost);
		initData.properties.setProperty("IceSSL.CertFile", defaultDir + "/c_rsa_nopass_ca1.pfx");
		// Don't specify the password.
		//props.setProperty("IceSSL.Password", "password");
		try
		{
		    Ice.Util.initialize(ref args, initData);
		    test(false);
		}
		catch(Ice.PluginInitializationException)
		{
		    // Expected.
		}
		catch(Ice.LocalException)
		{
		    test(false);
		}
	    }
	    Console.Out.WriteLine("ok");
	}
	finally
	{
	    store.Remove(caCert1);
	    store.Remove(caCert2);
	    store.Close();
	}

	factory.shutdown();
    }
}
