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
#include <Ice/System.h>
#include <Ice/SslException.h>
#include <Ice/SslExtension.h>
#include <TestCommon.h>
#include <Pinger.h>

using namespace std;

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    string ref = "pinger:ssl -p 12345 -t 2000";

    KeyManagerPrx km = KeyManagerPrx::checkedCast(communicator->stringToProxy("keyManager:tcp -p 12344 -t 2000"));

    Ice::ByteSeq serverTrustedCert;
    Ice::ByteSeq serverUntrustedCert;
    Ice::ByteSeq clientTrustedKey;
    Ice::ByteSeq clientTrustedCert;
    Ice::ByteSeq clientUntrustedKey;
    Ice::ByteSeq clientUntrustedCert;

    km->getServerCerts(serverTrustedCert,serverUntrustedCert);
    km->getTrustedClientKeys(clientTrustedKey,clientTrustedCert);
    km->getUntrustedClientKeys(clientUntrustedKey,clientUntrustedCert);

    IceSSL::SystemPtr sslSystem = communicator->getSslSystem();

    Ice::PropertiesPtr properties = communicator->getProperties();

    bool singleCertVerifier = false;
    if (properties->getProperty("Ice.SSL.Client.CertificateVerifier") == "singleCert")
    {
        singleCertVerifier = true;
    }

/*

// Note: This section is commented out because Ice is currently not destroying SslTransceivers
//       properly, resulting in cached connections (which cause the expected failure sections
//       to succeed, causing the test to fail).

    if (!singleCertVerifier)
    {
        cout << "client and server trusted, client using stock certificate... ";

        properties->setProperty("Ice.SSL.Client.CertPath","../certs");
        properties->setProperty("Ice.SSL.Client.Config", "sslconfig_6.xml");
        sslSystem->configure(IceSSL::Client);
        sslSystem->addTrustedCertificate(IceSSL::Client, serverTrustedCert);
        try
        {
            PingerPrx pinger = PingerPrx::checkedCast(communicator->stringToProxy(ref));
            pinger->ping();
            cout << "ok" << endl;
        }
        catch(const Ice::LocalException& localEx)
        {
            cout << localEx << endl;
            km->shutdown();
            test(false);
        }
    }
*/

    properties->setProperty("Ice.SSL.Client.CertPath","../certs");
    properties->setProperty("Ice.SSL.Client.Config", "sslconfig_7.xml");

    cout << "client and server do not trust each other... " << flush;

    // Neither Client nor Server will trust.
    sslSystem->configure(IceSSL::Client);
    sslSystem->addTrustedCertificate(IceSSL::Client, serverUntrustedCert);
    if (singleCertVerifier)
    {
        IceSSL::SslExtensionPtr sslExtension = communicator->getSslExtension();
        IceSSL::CertificateVerifierPtr certVerifier = sslExtension->getSingleCertVerifier(serverUntrustedCert);
        sslSystem->setCertificateVerifier(IceSSL::Client, certVerifier);
    }
    sslSystem->setRSAKeys(IceSSL::Client, clientUntrustedKey, clientUntrustedCert);
    try
    {
        PingerPrx pinger = PingerPrx::checkedCast(communicator->stringToProxy(ref));
        pinger->ping();
        km->shutdown();
        test(false);
    }
    catch(const IceSSL::CertificateVerificationException&)
    {
	cout << "ok" << endl;
    }
    catch(const Ice::LocalException&)
    {
        km->shutdown();
        test(false);
    }

    cout << "client trusted, server not trusted... " << flush;

    // Client will not trust Server, but Server will trust Client.
    sslSystem->setRSAKeys(IceSSL::Client, clientTrustedKey, clientTrustedCert);
    try
    {
        PingerPrx pinger = PingerPrx::checkedCast(communicator->stringToProxy(ref));
        pinger->ping();
        km->shutdown();
        test(false);
    }
    catch(const IceSSL::CertificateVerificationException&)
    {
	cout << "ok" << endl;
    }
    catch(const Ice::LocalException&)
    {
        km->shutdown();
        test(false);
    }

    cout << "client trusts server, server does not trust client... " << flush;

    // Client trusts, Server does not.
    sslSystem->configure(IceSSL::Client);
    sslSystem->addTrustedCertificate(IceSSL::Client, serverTrustedCert);
    if (singleCertVerifier)
    {
        IceSSL::SslExtensionPtr sslExtension = communicator->getSslExtension();
        IceSSL::CertificateVerifierPtr certVerifier = sslExtension->getSingleCertVerifier(serverTrustedCert);
        sslSystem->setCertificateVerifier(IceSSL::Client, certVerifier);
    }
    sslSystem->setRSAKeys(IceSSL::Client, clientUntrustedKey, clientUntrustedCert);
    try
    {
        PingerPrx pinger = PingerPrx::checkedCast(communicator->stringToProxy(ref));
        pinger->ping();
        km->shutdown();
        test(false);
    }
    catch(const IceSSL::ProtocolException&)
    {
        // Note: We expect that the server will send an alert 48 back to the client,
        //       generating this exception.
	cout << "ok" << endl;
    }
    catch(const Ice::LocalException&)
    {
        km->shutdown();
        test(false);
    }

    cout << "both client and server trust each other... " << flush;

    // Both Client and Server trust.
    sslSystem->setRSAKeys(IceSSL::Client, clientTrustedKey, clientTrustedCert);

    try
    {
        PingerPrx pinger = PingerPrx::checkedCast(communicator->stringToProxy(ref));
	pinger->ping();
	cout << "ok" << endl;
    }
    catch(const Ice::LocalException&)
    {
        km->shutdown();
        test(false);
    }

    cout << "shutting down... " << flush;
    km->shutdown();
    cout << "ok" << endl;

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
