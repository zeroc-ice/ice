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
#include <Ice/CertificateVerifier.h>
#include <Ice/CertificateVerifierOpenSSL.h>
#include <Ice/SslException.h>
#include <Ice/System.h>

using namespace std;
using namespace Ice;

//
// Certificate Verifier definitions
//

class BadCertificateVerifier : virtual public ::IceSSL::CertificateVerifier
{
public:
};

class GoodCertificateVerifier : virtual public ::IceSSL::OpenSSL::CertificateVerifier
{
public:
    virtual int verify(int, X509_STORE_CTX*, SSL*);
};

int
GoodCertificateVerifier::verify(int preVerifyOk, X509_STORE_CTX* certificateStore, SSL* sslConnection)
{
    return preVerifyOk;
}

//
// certificateVerifierClient definition
//

class certificateVerifierClient : public Application
{
public:

    virtual int run(int, char*[]);

private:
    PropertiesPtr _properties;
    IceSSL::SystemPtr _system;

    ::IceSSL::CertificateVerifierPtr _badVerifier;
    ::IceSSL::CertificateVerifierPtr _goodVerifier;

    void testExpectCertificateVerifierTypeException(IceSSL::ContextType, const IceSSL::CertificateVerifierPtr&);
    void testExpectNoException(IceSSL::ContextType, const IceSSL::CertificateVerifierPtr&);
};

void
certificateVerifierClient::testExpectCertificateVerifierTypeException(IceSSL::ContextType context,
                                                                      const IceSSL::CertificateVerifierPtr& verifier)
{
    try
    {
        _system->setCertificateVerifier(context, verifier);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::CertificateVerifierTypeException&)
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
certificateVerifierClient::testExpectNoException(IceSSL::ContextType context,
                                                 const IceSSL::CertificateVerifierPtr& verifier)
{
    try
    {
        _system->setCertificateVerifier(context, verifier);
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
main(int argc, char* argv[])
{
    certificateVerifierClient app;
    return app.main(argc, argv, "");
}

int
certificateVerifierClient::run(int argc, char* argv[])
{
    _properties = communicator()->getProperties();
    _system = communicator()->getSslSystem();

    _badVerifier  = new BadCertificateVerifier();
    _goodVerifier = new GoodCertificateVerifier();

    //
    // Testing IceSSL::Client context type.
    //

    std::cout << "Setting Certificate Verifiers on Client context." << std::endl;

    std::cout << "Setting verifier of wrong type... ";
    testExpectCertificateVerifierTypeException(IceSSL::Client, _badVerifier);

    std::cout << "Setting verifier of correct type... ";
    testExpectNoException(IceSSL::Client, _goodVerifier);

    //
    // Testing IceSSL::Server context type.
    //

    std::cout << "Setting Certificate Verifiers on Server context." << std::endl;

    std::cout << "Setting verifier of wrong type... ";
    testExpectCertificateVerifierTypeException(IceSSL::Server, _badVerifier);

    std::cout << "Setting verifier of correct type... ";
    testExpectNoException(IceSSL::Server, _goodVerifier);

    //
    // Testing IceSSL::ClientServer context type.
    //

    std::cout << "Setting Certificate Verifiers on Client and Server contexts." << std::endl;

    std::cout << "Setting verifier of wrong type... ";
    testExpectCertificateVerifierTypeException(IceSSL::ClientServer, _badVerifier);

    std::cout << "Setting verifier of correct type... ";
    testExpectNoException(IceSSL::ClientServer, _goodVerifier);

    return EXIT_SUCCESS;
}
