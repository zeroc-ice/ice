// **********************************************************************
//
// Copyright (c) 2002
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Application.h>
#include <Ice/SslException.h>
#include <Ice/RSACertificateGen.h>
#include <Ice/RSAKeyPair.h>
#include <Ice/System.h>
#include <IceUtil/Base64.h>

#include <fstream>

using namespace std;
using namespace Ice;

class certKeyParsingClient : public Application
{
public:

    virtual int run(int, char*[]);

    void testExpectCertificateAndPrivateKeyParseException(const std::string&, const std::string&);
    void testExpectCertificateAndPrivateKeyParseException(const Ice::ByteSeq&, const Ice::ByteSeq&);
    void testExpectPrivateKeyParseException(const std::string&, const std::string&);
    void testExpectPrivateKeyParseException(const Ice::ByteSeq&, const Ice::ByteSeq&);
    void testExpectCertificateParseException(const std::string&, const std::string&);
    void testExpectCertificateParseException(const Ice::ByteSeq&, const Ice::ByteSeq&);
    void testExpectContextNotConfiguredException(const std::string&, const std::string&);
    void testExpectContextNotConfiguredException(const Ice::ByteSeq&, const Ice::ByteSeq&);
    void testExpectCertificateKeyMatchException(const std::string&, const std::string&);
    void testExpectCertificateKeyMatchException(const Ice::ByteSeq&, const Ice::ByteSeq&);
    void testNoException(const std::string&, const std::string&);
    void testNoException(const Ice::ByteSeq&, const Ice::ByteSeq&);

private:
    PropertiesPtr _properties;
    IceSSL::SystemPtr _system;

    Ice::ByteSeq _gcert1;
    Ice::ByteSeq _gkey1;
    std::string _gcert1b64;
    std::string _gkey1b64;

    Ice::ByteSeq _gcert2;
    Ice::ByteSeq _gkey2;
    std::string _gcert2b64;
    std::string _gkey2b64;

    Ice::ByteSeq _badCert;
    Ice::ByteSeq _badKey;
    std::string _badCertb64;
    std::string _badKeyb64;
};

int
main(int argc, char* argv[])
{
    certKeyParsingClient app;
    return app.main(argc, argv, "");
}

void
certKeyParsingClient::testExpectCertificateAndPrivateKeyParseException(const std::string& key,
                                                                       const std::string& cert)
{
    try
    {
        _system->setRSAKeysBase64(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::CertificateParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectCertificateAndPrivateKeyParseException(const Ice::ByteSeq& key,
                                                                       const Ice::ByteSeq& cert)
{
    try
    {
        _system->setRSAKeys(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::CertificateParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectPrivateKeyParseException(const std::string& key,
                                                         const std::string& cert)
{
    try
    {
        _system->setRSAKeysBase64(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectPrivateKeyParseException(const Ice::ByteSeq& key,
                                                         const Ice::ByteSeq& cert)
{
    try
    {
        _system->setRSAKeys(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::PrivateKeyParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectCertificateParseException(const std::string& key,
                                                          const std::string& cert)
{
    try
    {
        _system->setRSAKeysBase64(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::CertificateParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectCertificateParseException(const Ice::ByteSeq& key,
                                                          const Ice::ByteSeq& cert)
{
    try
    {
        _system->setRSAKeys(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::CertificateParseException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectContextNotConfiguredException(const std::string& key,
                                                              const std::string& cert)
{
    try
    {
        _system->setRSAKeysBase64(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::ContextNotConfiguredException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectContextNotConfiguredException(const Ice::ByteSeq& key,
                                                              const Ice::ByteSeq& cert)
{
    try
    {
        _system->setRSAKeys(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::ContextNotConfiguredException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectCertificateKeyMatchException(const std::string& key,
                                                             const std::string& cert)
{
    try
    {
        _system->setRSAKeysBase64(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::CertificateKeyMatchException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (const IceUtil::Exception& iceUtilEx)
    {
        std::cout << "failed" << std::endl;
        std::cout << iceUtilEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testExpectCertificateKeyMatchException(const Ice::ByteSeq& key,
                                                             const Ice::ByteSeq& cert)
{
    try
    {
        _system->setRSAKeys(IceSSL::Client, key, cert);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::CertificateKeyMatchException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (const IceUtil::Exception& iceUtilEx)
    {
        std::cout << "failed" << std::endl;
        std::cout << iceUtilEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testNoException(const std::string& key, const std::string& cert)
{
    try
    {
        _system->setRSAKeysBase64(IceSSL::Client, key, cert);
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

void
certKeyParsingClient::testNoException(const Ice::ByteSeq& key, const Ice::ByteSeq& cert)
{
    try
    {
        _system->setRSAKeys(IceSSL::Client, key, cert);
        std::cout << "ok" << std::endl;
    }
    catch (const Ice::LocalException& localEx)
    {
        //
        // Any other exception is bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << localEx << std::endl;
        abort();
    }
    catch (...)
    {
        //
        // Unknown exceptions are always bad.
        //

        std::cout << "failed" << std::endl;
        std::cout << "Unknown exception." << std::endl;
        abort();
    }
}

int
certKeyParsingClient::run(int argc, char* argv[])
{
    _properties = communicator()->getProperties();
    _system = communicator()->getSslSystem();

    IceSSL::OpenSSL::RSACertificateGen certGen;

    IceSSL::OpenSSL::RSAKeyPairPtr goodKeyPair1;
    IceSSL::OpenSSL::RSAKeyPairPtr goodKeyPair2;

    goodKeyPair1 = certGen.loadKeyPair("../certs/goodKey_1.pem", "../certs/goodCert_1.pem");
    goodKeyPair2 = certGen.loadKeyPair("../certs/goodKey_2.pem", "../certs/goodCert_2.pem");

    goodKeyPair1->certToByteSeq(_gcert1);
    goodKeyPair1->keyToByteSeq(_gkey1);
    goodKeyPair1->certToBase64(_gcert1b64);
    goodKeyPair1->keyToBase64(_gkey1b64);

    goodKeyPair2->certToByteSeq(_gcert2);
    goodKeyPair2->keyToByteSeq(_gkey2);
    goodKeyPair2->certToBase64(_gcert2b64);
    goodKeyPair2->keyToBase64(_gkey2b64);

    std::ifstream keyStream("../certs/badKey.b64");
    std::ifstream certStream("../certs/badCert.b64");

    keyStream >> _badKeyb64;
    certStream >> _badCertb64;

    keyStream.close();
    certStream.close();

    _badKey = IceUtil::Base64::decode(_badKeyb64);
    _badCert = IceUtil::Base64::decode(_badCertb64);

    std::cout << "Testing certificate and key parsing failures." << std::endl;

    std::cout << "Bad private key and certificate (Base64)... ";
    testExpectCertificateAndPrivateKeyParseException(_badKeyb64, _badCertb64);

    std::cout << "Bad private key and certificate... ";
    testExpectCertificateAndPrivateKeyParseException(_badKey, _badCert);

    std::cout << "Bad private key and good certificate (Base64)... ";
    testExpectPrivateKeyParseException(_badKeyb64, _gcert1b64);

    std::cout << "Bad private key and good certificate... ";
    testExpectPrivateKeyParseException(_badKey, _gcert1);

    std::cout << "Good private key and bad certificate (Base64)... ";
    testExpectCertificateParseException(_gkey1b64, _badCertb64);

    std::cout << "Good private key and bad certificate... ";
    testExpectCertificateParseException(_gkey1, _badCert);
    
    std::cout << "Testing setting good certificates and keys on a unconfigured context." << std::endl;

    std::cout << "Good private key and certificate... ";
    testExpectContextNotConfiguredException(_gkey1, _gcert1);
    
    std::cout << "Good private key and certificate (Base64)... ";
    testExpectContextNotConfiguredException(_gkey1b64, _gcert1b64);

    std::cout << "Good private key and certificate (again)... ";
    testExpectContextNotConfiguredException(_gkey2, _gcert2);
    
    std::cout << "Good private key and certificate (Base64) (again)... ";
    testExpectContextNotConfiguredException(_gkey2b64, _gcert2b64);

    _properties->setProperty("Ice.SSL.Client.CertPath", "../certs");
    _properties->setProperty("Ice.SSL.Client.Config", "sslconfig_6.xml");
    _system->configure(IceSSL::Client);

    std::cout << "Testing mismatched certificates and keys failures on a configured context." << std::endl;

    std::cout << "Good private key and certificate, mismatched... ";
    testExpectCertificateKeyMatchException(_gkey1, _gcert2);
    
    std::cout << "Good private key and certificate, mismatched (Base64)... ";
    testExpectCertificateKeyMatchException(_gkey1b64, _gcert2b64);

    std::cout << "Good private key and certificate, mismatched (again)... ";
    testExpectCertificateKeyMatchException(_gkey2, _gcert1);
    
    std::cout << "Good private key and certificate, mismatched (Base64) (again)... ";
    testExpectCertificateKeyMatchException(_gkey2b64, _gcert1b64);

    std::cout << "Testing setting good certificates and keys on a configured context." << std::endl;

    std::cout << "Good private key and certificate... ";
    testNoException(_gkey1, _gcert1);
    
    std::cout << "Good private key and certificate (Base64)... ";
    testNoException(_gkey1b64, _gcert1b64);

    std::cout << "Good private key and certificate (again)... ";
    testNoException(_gkey2, _gcert2);
    
    std::cout << "Good private key and certificate (Base64) (again)... ";
    testNoException(_gkey2b64, _gcert2b64);


    return EXIT_SUCCESS;
}
