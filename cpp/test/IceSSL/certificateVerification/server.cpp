// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <Ice/RSACertificateGen.h>
#include <Ice/RSAKeyPair.h>
#include <Ice/SslExtension.h>
#include <Ice/System.h>
#include <Pinger.h>

using namespace std;


class KeyManagerI : public KeyManager
{

public:
    KeyManagerI(const IceSSL::OpenSSL::RSAKeyPairPtr&, const IceSSL::OpenSSL::RSAKeyPairPtr&,
                const IceSSL::OpenSSL::RSAKeyPairPtr&, const IceSSL::OpenSSL::RSAKeyPairPtr&,
                const Ice::CommunicatorPtr&);

    virtual void getServerCerts(Ice::ByteSeq&, Ice::ByteSeq&, const ::Ice::Current&);
    virtual void getTrustedClientKeys(Ice::ByteSeq&, Ice::ByteSeq&, const ::Ice::Current&);
    virtual void getUntrustedClientKeys(Ice::ByteSeq&, Ice::ByteSeq&, const ::Ice::Current&);
    virtual void shutdown(const ::Ice::Current&);

protected:
    IceSSL::OpenSSL::RSAKeyPairPtr _serverTrusted;
    IceSSL::OpenSSL::RSAKeyPairPtr _serverUntrusted;
    IceSSL::OpenSSL::RSAKeyPairPtr _clientTrusted;
    IceSSL::OpenSSL::RSAKeyPairPtr _clientUntrusted;
    Ice::CommunicatorPtr _communicator;
};

KeyManagerI::KeyManagerI(const IceSSL::OpenSSL::RSAKeyPairPtr& serverTrusted,
                         const IceSSL::OpenSSL::RSAKeyPairPtr& serverUntrusted,
                         const IceSSL::OpenSSL::RSAKeyPairPtr& clientTrusted,
                         const IceSSL::OpenSSL::RSAKeyPairPtr& clientUntrusted,
                         const Ice::CommunicatorPtr& communicator) :
            _serverTrusted(serverTrusted), _serverUntrusted(serverUntrusted),
            _clientTrusted(clientTrusted), _clientUntrusted(clientUntrusted),
            _communicator(communicator)
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
KeyManagerI::shutdown(const ::Ice::Current&)
{
    _communicator->shutdown();
}

class PingerI : public Pinger
{

public:
    PingerI();
    virtual void ping(const ::Ice::Current&);
};

PingerI::PingerI()
{
}

void
PingerI::ping(const ::Ice::Current&)
{
    std::cout << "ping." << std::endl;
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    Ice::PropertiesPtr properties = communicator->getProperties();

    // Test crashes if I put this in.
    // properties->setProperty("Ice.ConnectionWarnings", "0");

    properties->setProperty("Ice.SSL.Client.CertPath","../certs");
    properties->setProperty("Ice.SSL.Client.Config", "sslconfig_8.xml");

    IceSSL::SystemPtr sslSystem = communicator->getSslSystem();
    sslSystem->configure(IceSSL::Server);

    IceSSL::OpenSSL::RSACertificateGen certGen;

    IceSSL::OpenSSL::RSACertificateGenContext certGenContext;

    // Base setup.
    certGenContext.setCountry("US");
    certGenContext.setStateProvince("DC");
    certGenContext.setLocality("Washington");
    certGenContext.setOrganization("Some Company Inc.");
    certGenContext.setOrgainizationalUnit("Sales");
    certGenContext.setBitStrength(1024);
    certGenContext.setSecondsValid(IceSSL::OpenSSL::RSACertificateGenContext::hoursToSeconds(1));

    IceSSL::OpenSSL::RSAKeyPairPtr serverTrusted;
    IceSSL::OpenSSL::RSAKeyPairPtr serverUntrusted;
    IceSSL::OpenSSL::RSAKeyPairPtr clientTrusted;
    IceSSL::OpenSSL::RSAKeyPairPtr clientUntrusted;

    certGenContext.setCommonName("Server Trusted");
    serverTrusted = certGen.generate(certGenContext);

    certGenContext.setCommonName("Server Untrusted");
    serverUntrusted = certGen.generate(certGenContext);

    certGenContext.setCommonName("Client Trusted");
    clientTrusted = certGen.generate(certGenContext);

    certGenContext.setCommonName("Client Untrusted");
    clientUntrusted = certGen.generate(certGenContext);

    Ice::ObjectPtr object = new KeyManagerI(serverTrusted, serverUntrusted,
                                            clientTrusted, clientUntrusted,
                                            communicator);

    Ice::ByteSeq trustedCertificate;
    Ice::ByteSeq serverCertificate;
    Ice::ByteSeq serverKey;

    clientTrusted->certToByteSeq(trustedCertificate);
    serverTrusted->certToByteSeq(serverCertificate);
    serverTrusted->keyToByteSeq(serverKey);

    sslSystem->addTrustedCertificate(IceSSL::Server, trustedCertificate);
    sslSystem->setRSAKeys(IceSSL::Server, serverKey, serverCertificate);

    if (properties->getProperty("Ice.SSL.Server.CertificateVerifier") == "singleCert")
    {
        IceSSL::SslExtensionPtr sslExtension = communicator->getSslExtension();
        IceSSL::CertificateVerifierPtr certVerifier = sslExtension->getSingleCertVerifier(trustedCertificate);
        sslSystem->setCertificateVerifier(IceSSL::Server, certVerifier);
    }

    string kmEndpts = "tcp -p 12344 -t 2000";
    Ice::ObjectAdapterPtr kmAdapter = communicator->createObjectAdapterWithEndpoints("KeyManagerAdapter", kmEndpts);
    kmAdapter->add(object, Ice::stringToIdentity("keyManager"));
    kmAdapter->activate();

    string endpts = "ssl -p 12345 -t 2000";
    Ice::ObjectAdapterPtr adapter = communicator->createObjectAdapterWithEndpoints("PingerAdapter", endpts);
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
	status = run(argc, argv, communicator);

    }
    catch(const Ice::Exception& ex)
    {
	cerr << ex << endl;
	status = EXIT_FAILURE;
    }

    if (communicator)
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
