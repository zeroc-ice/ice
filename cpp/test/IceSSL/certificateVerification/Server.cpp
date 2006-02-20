// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceSSL/RSACertificateGen.h>
#include <IceSSL/RSAKeyPair.h>
#include <IceSSL/Plugin.h>
#include <Pinger.h>

using namespace std;
using namespace Test;

class KeyManagerI : public KeyManager
{

public:

    KeyManagerI(const IceSSL::RSAKeyPairPtr&, const IceSSL::RSAKeyPairPtr&,
                const IceSSL::RSAKeyPairPtr&, const IceSSL::RSAKeyPairPtr&);

    virtual void getServerCerts(Ice::ByteSeq&, Ice::ByteSeq&, const ::Ice::Current&);
    virtual void getTrustedClientKeys(Ice::ByteSeq&, Ice::ByteSeq&, const ::Ice::Current&);
    virtual void getUntrustedClientKeys(Ice::ByteSeq&, Ice::ByteSeq&, const ::Ice::Current&);
    virtual void shutdown(const ::Ice::Current&);

protected:

    const IceSSL::RSAKeyPairPtr _serverTrusted;
    const IceSSL::RSAKeyPairPtr _serverUntrusted;
    const IceSSL::RSAKeyPairPtr _clientTrusted;
    const IceSSL::RSAKeyPairPtr _clientUntrusted;
};

KeyManagerI::KeyManagerI(const IceSSL::RSAKeyPairPtr& serverTrusted,
                         const IceSSL::RSAKeyPairPtr& serverUntrusted,
                         const IceSSL::RSAKeyPairPtr& clientTrusted,
                         const IceSSL::RSAKeyPairPtr& clientUntrusted) :
            _serverTrusted(serverTrusted), _serverUntrusted(serverUntrusted),
            _clientTrusted(clientTrusted), _clientUntrusted(clientUntrusted)
{
}

void
KeyManagerI::getServerCerts(Ice::ByteSeq& trusted, Ice::ByteSeq& untrusted, const ::Ice::Current&)
{
    _serverTrusted->certToByteSeq(trusted);
    _serverUntrusted->certToByteSeq(untrusted);
}

void
KeyManagerI::getTrustedClientKeys(Ice::ByteSeq& key, Ice::ByteSeq& cert, const ::Ice::Current&)
{
    _clientTrusted->keyToByteSeq(key);
    _clientTrusted->certToByteSeq(cert);
}

void
KeyManagerI::getUntrustedClientKeys(Ice::ByteSeq& key, Ice::ByteSeq& cert, const ::Ice::Current&)
{
    _clientUntrusted->keyToByteSeq(key);
    _clientUntrusted->certToByteSeq(cert);
}

void
KeyManagerI::shutdown(const ::Ice::Current& c)
{
    c.adapter->getCommunicator()->shutdown();
}

class PingerI : public Pinger
{
public:

    PingerI() { }
};

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    std::string certPath = properties->getProperty("TestSSL.Server.CertPath");
    properties->setProperty("IceSSL.Server.CertPath", certPath);

    properties->setProperty("Ice.Warn.Connections", "0");
    properties->setProperty("IceSSL.Server.Config", "sslconfig_8.xml");

    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);
    sslPlugin->configure(IceSSL::Server);

    IceSSL::RSACertificateGen certGen;

    IceSSL::RSACertificateGenContext certGenContext;

    // Base setup.
    certGenContext.setCountry("US");
    certGenContext.setStateProvince("DC");
    certGenContext.setLocality("Washington");
    certGenContext.setOrganization("Some Company Inc.");
    certGenContext.setOrgainizationalUnit("Sales");
    certGenContext.setBitStrength(1024);
    certGenContext.setSecondsValid(IceSSL::RSACertificateGenContext::hoursToSeconds(1));

    IceSSL::RSAKeyPairPtr serverTrusted;
    IceSSL::RSAKeyPairPtr serverUntrusted;
    IceSSL::RSAKeyPairPtr clientTrusted;
    IceSSL::RSAKeyPairPtr clientUntrusted;

    certGenContext.setCommonName("Server Trusted");
    serverTrusted = certGen.generate(certGenContext);

    certGenContext.setCommonName("Server Untrusted");
    serverUntrusted = certGen.generate(certGenContext);

    certGenContext.setCommonName("Client Trusted");
    clientTrusted = certGen.generate(certGenContext);

    certGenContext.setCommonName("Client Untrusted");
    clientUntrusted = certGen.generate(certGenContext);

    Ice::ObjectPtr object = new KeyManagerI(serverTrusted, serverUntrusted, clientTrusted, clientUntrusted);

    Ice::ByteSeq trustedCertificate;
    Ice::ByteSeq serverCertificate;
    Ice::ByteSeq serverKey;

    clientTrusted->certToByteSeq(trustedCertificate);
    serverTrusted->certToByteSeq(serverCertificate);
    serverTrusted->keyToByteSeq(serverKey);

    sslPlugin->addTrustedCertificate(IceSSL::Server, trustedCertificate);
    sslPlugin->setRSAKeys(IceSSL::Server, serverKey, serverCertificate);

    if(properties->getProperty("TestSSL.Server.CertificateVerifier") == "singleCert")
    {
        IceSSL::CertificateVerifierPtr certVerifier = sslPlugin->getSingleCertVerifier(trustedCertificate);
        sslPlugin->setCertificateVerifier(IceSSL::Server, certVerifier);
    }

    properties->setProperty("KeyManagerAdapter.Endpoints", "tcp -p 12344 -t 10000");
    bool printAdapterReady = properties->getPropertyAsInt("Ice.PrintAdapterReady") > 0;
    properties->setProperty("Ice.PrintAdapterReady", "0");
    Ice::ObjectAdapterPtr kmAdapter = communicator->createObjectAdapter("KeyManagerAdapter");
    kmAdapter->add(object, Ice::stringToIdentity("keyManager"));
    kmAdapter->activate();

    const string pingerEndpoints =
	"ssl -p 12010 -t 10000"
	":ssl -p 12011 -t 10000"
	":ssl -p 12347 -t 10000"
	":ssl -p 12348 -t 10000"
	":ssl -p 12349 -t 10000";
    if(printAdapterReady)
    {
	properties->setProperty("Ice.PrintAdapterReady", "1");
    }
    properties->setProperty("PingerAdapter.Endpoints", pingerEndpoints);
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapter("PingerAdapter");
    adapter->add(new PingerI(), Ice::stringToIdentity("pinger"));
    adapter->activate();
    communicator->waitForShutdown();
    return EXIT_SUCCESS;
}

int
main(int argc, char* argv[])
{
    int status;
    Ice::CommunicatorPtr communicator;

    try
    {
	communicator = Ice::initialize(argc, argv);
        Ice::PropertiesPtr properties = communicator->getProperties();
        Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
        args = properties->parseCommandLineOptions("TestSSL", args);
        Ice::stringSeqToArgs(args, argc, argv);
	status = run(argc, argv, communicator);
    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if(communicator)
    {
	try
	{
	    communicator->destroy();
	}
	catch(const Ice::Exception& ex)
	{
	    cerr << ex << endl;
	    status = EXIT_FAILURE;
	}
    }

    return status;
}
