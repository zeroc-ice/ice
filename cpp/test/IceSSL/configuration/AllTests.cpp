// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/Plugin.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Ice;

class PasswordPromptI : public IceSSL::PasswordPrompt
{
public:

    PasswordPromptI(const string& password) : _password(password), _count(0)
    {
    }

    virtual string getPassword()
    {
	++_count; 
	return _password;
    }

    int count() const
    {
	return _count;
    }

private:

    string _password;
    int _count;
};
typedef IceUtil::Handle<PasswordPromptI> PasswordPromptIPtr;

class CertificateVerifierI : public IceSSL::CertificateVerifier
{
public:

    CertificateVerifierI()
    {
	reset();
    }

    virtual void verify(IceSSL::VerifyInfo& info)
    {
	if(info.cert)
	{
	    test(find(info.dnsNames.begin(), info.dnsNames.end(), "server") != info.dnsNames.end());
	    test(find(info.ipAddresses.begin(), info.ipAddresses.end(), "127.0.0.1") != info.ipAddresses.end());
	}

	_incoming = info.incoming;
	_hadCert = info.cert != 0;
	_invoked = true;

	if(_throwException)
	{
	    throw SecurityException(__FILE__, __LINE__);
	}
    }

    void reset()
    {
	_throwException = false;
       	_invoked = false;
	_incoming = false;
	_hadCert = false;
    }

    void throwException(bool b)
    {
	_throwException = b;
    }

    bool invoked() const
    {
	return _invoked;
    }

    bool incoming() const
    {
	return _incoming;
    }

    bool hadCert() const
    {
	return _hadCert;
    }

private:

    bool _throwException;
    bool _invoked;
    bool _incoming;
    bool _hadCert;
};
typedef IceUtil::Handle<CertificateVerifierI> CertificateVerifierIPtr;

static PropertiesPtr
createClientProps(const string& defaultHost)
{
    PropertiesPtr result = createProperties();
    result->setProperty("Ice.Plugin.IceSSL", "IceSSL:create");
    if(!defaultHost.empty())
    {
	result->setProperty("Ice.Default.Host", defaultHost);
    }
    return result;
}

static Test::Properties
createServerProps(const string& defaultHost)
{
    Test::Properties result;
    result["Ice.Plugin.IceSSL"] = "IceSSL:create";
    if(!defaultHost.empty())
    {
	result["Ice.Default.Host"] = defaultHost;
    }
    return result;
}

void
allTests(const CommunicatorPtr& communicator, const string& testDir)
{
    string factoryRef = "factory:tcp -p 12010 -t 10000";
    ObjectPrx base = communicator->stringToProxy(factoryRef);
    test(base);
    Test::ServerFactoryPrx factory = Test::ServerFactoryPrx::checkedCast(base);

    string defaultHost = communicator->getProperties()->getProperty("Ice.Default.Host");
    string defaultDir = testDir + "/../certs";
#ifdef _WIN32
    string sep = ";";
#else
    string sep = ":";
#endif

    int argc = 0;
    char* argv[] = { "" };

    cout << "testing manual initialization... " << flush;
    {
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.DelayInit", "1");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	ObjectPrx p = comm->stringToProxy("dummy:ssl -p 9999");
	try
	{
	    p->ice_ping();
	    test(false);
	}
	catch(const PluginInitializationException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	comm->destroy();
    }
    {
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.DelayInit", "1");
	props->setProperty("IceSSL.Client.Ciphers", "ADH");
	props->setProperty("IceSSL.Client.VerifyPeer", "0");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	IceSSL::PluginPtr plugin =
	    IceSSL::PluginPtr::dynamicCast(comm->getPluginManager()->getPlugin("IceSSL"));
	test(plugin);
	plugin->initialize();
	ObjectPrx obj = comm->stringToProxy(factoryRef);
	test(obj);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(obj);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.Ciphers"] = "ADH";
	d["IceSSL.Server.VerifyPeer"] = "0";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing certificate verification... " << flush;
    {
	//
	// Test IceSSL.Server.VerifyPeer=0. Client does not have a certificate,
	// but it still verifies the server's.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.VerifyPeer", "0");
	props->setProperty("IceSSL.Client.CertAuthFile", "cacert1.pem");
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "0";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	//
	// Test IceSSL.Server.VerifyPeer=1. Client does not have a certificate.
	//
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "1";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	//
	// Test IceSSL.Server.VerifyPeer=2. This should fail because the client
	// does not supply a certificate.
	//
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "2";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
#ifdef _WIN32
	catch(const ConnectionLostException&)
	{
	    // Expected.
	}
#endif
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	comm->destroy();

	//
	// Test IceSSL.Server.VerifyPeer=1. Client has a certificate.
	//
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca1_priv.pem");
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "1";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	//
	// Test IceSSL.Server.VerifyPeer=2. Client has a certificate.
	//
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "2";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	comm->destroy();

	//
	// Test IceSSL.Server.VerifyPeer=1. This should fail because the
	// client doesn't trust the server's CA.
	//
	props->setProperty("IceSSL.Client.CertAuthFile", "cacert2.pem");
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca2_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca2_priv.pem");
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "1";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
#ifdef _WIN32
	catch(const ConnectionLostException&)
	{
	    // Expected.
	}
#endif
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	comm->destroy();

	//
	// Test IceSSL.Server.VerifyPeer=1. This should fail because the
	// server doesn't trust the client's CA.
	//
	props->setProperty("IceSSL.Client.CertAuthFile", "cacert1.pem");
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca2_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca2_priv.pem");
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.VerifyPeer"] = "1";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
#ifdef _WIN32
	catch(const ConnectionLostException&)
	{
	    // Expected.
	}
#endif
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);

	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing custom certificate verifier... " << flush;
    {
	//
	// ADH is allowed but will not have a certificate.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.Ciphers", "ADH");
	props->setProperty("IceSSL.Client.VerifyPeer", "0");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	IceSSL::PluginPtr plugin =
	    IceSSL::PluginPtr::dynamicCast(comm->getPluginManager()->getPlugin("IceSSL"));
	test(plugin);
	CertificateVerifierIPtr verifier = new CertificateVerifierI;
	plugin->setCertificateVerifier(verifier);

	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.Ciphers"] = "ADH";
	d["IceSSL.Server.VerifyPeer"] = "0";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	test(verifier->invoked());
	test(!verifier->incoming());
	test(!verifier->hadCert());

	//
	// Have the verifier raise an exception. Close the connection explicitly
	// to force a new connection to be established.
	//
	verifier->reset();
	verifier->throwException(true);
	server->ice_connection()->close(false);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const SecurityException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	test(verifier->invoked());
	test(!verifier->incoming());
	test(!verifier->hadCert());
	verifier->throwException(false);
	fact->destroyServer(server);

	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing protocols... " << flush;
    {
	//
	// This should fail because the client and server have no protocol
	// in common.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.Ciphers", "ADH");
	props->setProperty("IceSSL.Client.VerifyPeer", "0");
	props->setProperty("IceSSL.Client.Protocols", "ssl3");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.Ciphers"] = "ADH";
	d["IceSSL.Server.VerifyPeer"] = "0";
	d["IceSSL.Server.Protocols"] = "tls1";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
#ifdef _WIN32
	catch(const ProtocolException&)
	{
	    // Expected.
	}
#else
	catch(const ConnectionLostException&)
	{
	    // Expected.
	}
#endif
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();

	//
	// This should succeed.
	//
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.Ciphers"] = "ADH";
	d["IceSSL.Server.VerifyPeer"] = "0";
	d["IceSSL.Server.Protocols"] = "tls1, ssl3";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing expired certificates... " << flush;
    {
	//
	// This should fail because the server's certificate is expired.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertAuthFile", "cacert1.pem");
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca1_priv.pem");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_exp_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_exp_priv.pem";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();

	//
	// This should fail because the client's certificate is expired.
	//
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca1_exp_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca1_exp_priv.pem");
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertAuthFile"] = "cacert1.pem";
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing CA certificate directory... " << flush;
    {
	//
	// Don't specify CertAuthFile explicitly; we let OpenSSL find the CA
	// certificate in the default directory.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca1_priv.pem");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing password prompt... " << flush;
    {
	//
	// Use the correct password.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_pass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_pass_ca1_priv.pem");
	props->setProperty("IceSSL.DelayInit", "1");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	IceSSL::PluginPtr plugin =
	    IceSSL::PluginPtr::dynamicCast(comm->getPluginManager()->getPlugin("IceSSL"));
	test(plugin);
	PasswordPromptIPtr prompt = new PasswordPromptI("client");
	plugin->setPasswordPrompt(prompt);
	plugin->initialize();
	test(prompt->count() == 1);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();

	//
	// Use an incorrect password and check that retries are attempted.
	//
	props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_pass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_pass_ca1_priv.pem");
	props->setProperty("IceSSL.Client.PasswordRetryMax", "4");
	props->setProperty("IceSSL.DelayInit", "1");
	comm = initializeWithProperties(argc, argv, props);
	plugin = IceSSL::PluginPtr::dynamicCast(comm->getPluginManager()->getPlugin("IceSSL"));
	test(plugin);
	prompt = new PasswordPromptI("invalid");
	plugin->setPasswordPrompt(prompt);
	try
	{
	    plugin->initialize();
	}
	catch(const PluginInitializationException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	test(prompt->count() == 4);
	comm->destroy();
    }
    cout << "ok" << endl;

    cout << "testing ciphers... " << flush;
    {
	//
	// The server has a certificate but the client doesn't. They should
	// negotiate to use ADH since we explicitly enable it.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.Ciphers", "ADH");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.Ciphers"] = "RSA:ADH";
	d["IceSSL.Server.VerifyPeer"] = "1";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    {
	//
	// Configure a server with RSA and DSA certificates.
	//
	// First try a client with a DSA certificate.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertFile", "c_dsa_nopass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_dsa_nopass_ca1_priv.pem");
	props->setProperty("IceSSL.Client.Ciphers", "DEFAULT:DSS");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem" + sep + "s_dsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem" + sep + "s_dsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.Ciphers"] = "DEFAULT:DSS";
	d["IceSSL.Server.VerifyPeer"] = "1";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();

	//
	// Next try a client with an RSA certificate.
	//
	props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertFile", "c_rsa_nopass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_rsa_nopass_ca1_priv.pem");
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem" + sep + "s_dsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem" + sep + "s_dsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.Ciphers"] = "DEFAULT:DSS";
	d["IceSSL.Server.VerifyPeer"] = "1";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();

	//
	// Next try a client with ADH. This should fail.
	//
	props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.Ciphers", "ADH");
	comm = initializeWithProperties(argc, argv, props);
	fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem" + sep + "s_dsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem" + sep + "s_dsa_nopass_ca1_priv.pem";
	d["IceSSL.Server.Ciphers"] = "DEFAULT:DSS";
	d["IceSSL.Server.VerifyPeer"] = "1";
	server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    {
	//
	// Configure a server with RSA and a client with DSA. This should fail.
	//
	PropertiesPtr props = createClientProps(defaultHost);
	props->setProperty("IceSSL.Client.DefaultDir", defaultDir);
	props->setProperty("IceSSL.Client.CertFile", "c_dsa_nopass_ca1_pub.pem");
	props->setProperty("IceSSL.Client.KeyFile", "c_dsa_nopass_ca1_priv.pem");
	props->setProperty("IceSSL.Client.Ciphers", "DSS");
	CommunicatorPtr comm = initializeWithProperties(argc, argv, props);
	Test::ServerFactoryPrx fact = Test::ServerFactoryPrx::checkedCast(comm->stringToProxy(factoryRef));
	test(fact);
	Test::Properties d = createServerProps(defaultHost);
	d["IceSSL.Server.DefaultDir"] = defaultDir;
	d["IceSSL.Server.CertFile"] = "s_rsa_nopass_ca1_pub.pem";
	d["IceSSL.Server.KeyFile"] = "s_rsa_nopass_ca1_priv.pem";
	Test::ServerPrx server = fact->createServer(d);
	try
	{
	    server->ice_ping();
	    test(false);
	}
	catch(const ProtocolException&)
	{
	    // Expected.
	}
	catch(const LocalException&)
	{
	    test(false);
	}
	fact->destroyServer(server);
	comm->destroy();
    }
    cout << "ok" << endl;

    factory->shutdown();
}
