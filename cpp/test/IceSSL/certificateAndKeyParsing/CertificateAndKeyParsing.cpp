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
#include <TestCommon.h>
#include <Ice/SslException.h>
#include <Ice/RSACertificateGen.h>
#include <Ice/RSAKeyPair.h>
#include <Ice/System.h>
#include <IceUtil/Base64.h>

#include <fstream>

using namespace std;
using namespace Ice;

void
testExpectCertificateAndPrivateKeyParseException(const IceSSL::SystemPtr& system,
                                                 const std::string& key,
                                                 const std::string& cert)
{
    try
    {
        system->setRSAKeysBase64(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::CertificateParseException&)
    {
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateAndPrivateKeyParseException(const IceSSL::SystemPtr& system,
                                                 const Ice::ByteSeq& key,
                                                 const Ice::ByteSeq& cert)
{
    try
    {
        system->setRSAKeys(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::CertificateParseException&)
    {
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectPrivateKeyParseException(const IceSSL::SystemPtr& system, const std::string& key, const std::string& cert)
{
    try
    {
        system->setRSAKeysBase64(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectPrivateKeyParseException(const IceSSL::SystemPtr& system, const Ice::ByteSeq& key, const Ice::ByteSeq& cert)
{
    try
    {
        system->setRSAKeys(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateParseException(const IceSSL::SystemPtr& system, const std::string& key, const std::string& cert)
{
    try
    {
        system->setRSAKeysBase64(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::CertificateParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateParseException(const IceSSL::SystemPtr& system,
                                    const Ice::ByteSeq& key,
                                    const Ice::ByteSeq& cert)
{
    try
    {
        system->setRSAKeys(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::CertificateParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateParseException(const IceSSL::SystemPtr& system, const std::string& cert)
{
    try
    {
        system->addTrustedCertificateBase64(IceSSL::Client, cert);
        test(false);
    }
    catch (const IceSSL::CertificateParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateParseException(const IceSSL::SystemPtr& system, const Ice::ByteSeq& cert)
{
    try
    {
        system->addTrustedCertificate(IceSSL::Client, cert);
        test(false);
    }
    catch (const IceSSL::CertificateParseException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectContextNotConfiguredException(const IceSSL::SystemPtr& system,
                                        const std::string& key,
                                        const std::string& cert)
{
    try
    {
        system->setRSAKeysBase64(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::OpenSSL::ContextNotConfiguredException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectContextNotConfiguredException(const IceSSL::SystemPtr& system,
                                        const Ice::ByteSeq& key,
                                        const Ice::ByteSeq& cert)
{
    try
    {
        system->setRSAKeys(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::OpenSSL::ContextNotConfiguredException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectContextNotConfiguredException(const IceSSL::SystemPtr& system, const std::string& cert)
{
    try
    {
        system->addTrustedCertificateBase64(IceSSL::Client, cert);
        test(false);
    }
    catch (const IceSSL::OpenSSL::ContextNotConfiguredException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectContextNotConfiguredException(const IceSSL::SystemPtr& system, const Ice::ByteSeq& cert)
{
    try
    {
        system->addTrustedCertificate(IceSSL::Client, cert);
        test(false);
    }
    catch (const IceSSL::OpenSSL::ContextNotConfiguredException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateKeyMatchException(const IceSSL::SystemPtr& system,
                                       const std::string& key,
                                       const std::string& cert)
{
    try
    {
        system->setRSAKeysBase64(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::OpenSSL::CertificateKeyMatchException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testExpectCertificateKeyMatchException(const IceSSL::SystemPtr& system,
                                       const Ice::ByteSeq& key,
                                       const Ice::ByteSeq& cert)
{
    try
    {
        system->setRSAKeys(IceSSL::Client, key, cert);
        test(false);
    }
    catch (const IceSSL::OpenSSL::CertificateKeyMatchException&)
    {
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testNoException(const IceSSL::SystemPtr& system, const std::string& key, const std::string& cert)
{
    try
    {
        system->setRSAKeysBase64(IceSSL::Client, key, cert);
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testNoException(const IceSSL::SystemPtr& system, const Ice::ByteSeq& key, const Ice::ByteSeq& cert)
{
    try
    {
        system->setRSAKeys(IceSSL::Client, key, cert);
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testNoException(const IceSSL::SystemPtr& system, const std::string& cert)
{
    try
    {
        system->addTrustedCertificateBase64(IceSSL::Client, cert);
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

void
testNoException(const IceSSL::SystemPtr& system, const Ice::ByteSeq& cert)
{
    try
    {
        system->addTrustedCertificate(IceSSL::Client, cert);
    }
    catch (const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }

    std::cout << "ok" << std::endl;
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    PropertiesPtr properties = communicator->getProperties();
    IceSSL::SystemPtr system = communicator->getSslSystem();

    IceSSL::OpenSSL::RSACertificateGen certGen;

    IceSSL::OpenSSL::RSAKeyPairPtr goodKeyPair1;
    IceSSL::OpenSSL::RSAKeyPairPtr goodKeyPair2;

    goodKeyPair1 = certGen.loadKeyPair("../certs/goodKey_1.pem", "../certs/goodCert_1.pem");
    goodKeyPair2 = certGen.loadKeyPair("../certs/goodKey_2.pem", "../certs/goodCert_2.pem");

    Ice::ByteSeq gcert1;
    Ice::ByteSeq gkey1;
    std::string gcert1b64;
    std::string gkey1b64;

    Ice::ByteSeq gcert2;
    Ice::ByteSeq gkey2;
    std::string gcert2b64;
    std::string gkey2b64;

    Ice::ByteSeq badCert;
    Ice::ByteSeq badKey;
    std::string badCertb64;
    std::string badKeyb64;

    goodKeyPair1->certToByteSeq(gcert1);
    goodKeyPair1->keyToByteSeq(gkey1);
    goodKeyPair1->certToBase64(gcert1b64);
    goodKeyPair1->keyToBase64(gkey1b64);

    goodKeyPair2->certToByteSeq(gcert2);
    goodKeyPair2->keyToByteSeq(gkey2);
    goodKeyPair2->certToBase64(gcert2b64);
    goodKeyPair2->keyToBase64(gkey2b64);

    std::ifstream keyStream("../certs/badKey.b64");
    std::ifstream certStream("../certs/badCert.b64");

    keyStream >> badKeyb64;
    certStream >> badCertb64;

    keyStream.close();
    certStream.close();

    badKey = IceUtil::Base64::decode(badKeyb64);
    badCert = IceUtil::Base64::decode(badCertb64);

    std::cout << "Testing certificate and key parsing failures." << std::endl;

    std::cout << "Bad private key and certificate (Base64)... " << std::flush;
    testExpectCertificateAndPrivateKeyParseException(system, badKeyb64, badCertb64);

    std::cout << "Bad private key and certificate... " << std::flush;
    testExpectCertificateAndPrivateKeyParseException(system, badKey, badCert);

    std::cout << "Bad private key and good certificate (Base64)... " << std::flush;
    testExpectPrivateKeyParseException(system, badKeyb64, gcert1b64);

    std::cout << "Bad private key and good certificate... " << std::flush;
    testExpectPrivateKeyParseException(system, badKey, gcert1);

    std::cout << "Good private key and bad certificate (Base64)... " << std::flush;
    testExpectCertificateParseException(system, gkey1b64, badCertb64);

    std::cout << "Good private key and bad certificate... " << std::flush;
    testExpectCertificateParseException(system, gkey1, badCert);

    std::cout << "Bad certificate as a trusted certificate... " << std::flush;
    testExpectCertificateParseException(system, badCert);
    
    std::cout << "Bad certificate as a trusted certificate (Base64)... " << std::flush;
    testExpectCertificateParseException(system, badCertb64);
    
    std::cout << "Testing setting good certificates and keys on a unconfigured context." << std::endl;

    std::cout << "Good private key and certificate... " << std::flush;
    testExpectContextNotConfiguredException(system, gkey1, gcert1);
    
    std::cout << "Good private key and certificate (Base64)... " << std::flush;
    testExpectContextNotConfiguredException(system, gkey1b64, gcert1b64);

    std::cout << "Good private key and certificate (again)... " << std::flush;
    testExpectContextNotConfiguredException(system, gkey2, gcert2);
    
    std::cout << "Good private key and certificate (Base64) (again)... " << std::flush;
    testExpectContextNotConfiguredException(system, gkey2b64, gcert2b64);

    std::cout << "Good certificate as a trusted certificate... " << std::flush;
    testExpectContextNotConfiguredException(system, gcert1);
    
    std::cout << "Good certificate as a trusted certificate (Base64)... " << std::flush;
    testExpectContextNotConfiguredException(system, gcert1b64);

    properties->setProperty("Ice.SSL.Client.CertPath", "../certs");
    properties->setProperty("Ice.SSL.Client.Config", "sslconfig_6.xml");
    system->configure(IceSSL::Client);

    std::cout << "Testing mismatched certificates and keys failures on a configured context." << std::endl;

    std::cout << "Good private key and certificate, mismatched... " << std::flush;
    testExpectCertificateKeyMatchException(system, gkey1, gcert2);
    
    std::cout << "Good private key and certificate, mismatched (Base64)... " << std::flush;
    testExpectCertificateKeyMatchException(system, gkey1b64, gcert2b64);

    std::cout << "Good private key and certificate, mismatched (again)... " << std::flush;
    testExpectCertificateKeyMatchException(system, gkey2, gcert1);
    
    std::cout << "Good private key and certificate, mismatched (Base64) (again)... " << std::flush;
    testExpectCertificateKeyMatchException(system, gkey2b64, gcert1b64);

    std::cout << "Testing setting good certificates and keys on a configured context." << std::endl;

    std::cout << "Good private key and certificate... " << std::flush;
    testNoException(system, gkey1, gcert1);
    
    std::cout << "Good private key and certificate (Base64)... " << std::flush;
    testNoException(system, gkey1b64, gcert1b64);

    std::cout << "Good private key and certificate (again)... " << std::flush;
    testNoException(system, gkey2, gcert2);
    
    std::cout << "Good private key and certificate (Base64) (again)... " << std::flush;
    testNoException(system, gkey2b64, gcert2b64);

    std::cout << "Good certificate as trusted certificate... " << std::flush;
    testNoException(system, gcert1);
    
    std::cout << "Good certificate as trusted certificate (Base64)... " << std::flush;
    testNoException(system, gcert2b64);

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
