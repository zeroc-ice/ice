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

void
testExpectCertificateVerifierTypeException(const IceSSL::SystemPtr& system,
                                           IceSSL::ContextType context,
                                           const IceSSL::CertificateVerifierPtr& verifier)
{
    try
    {
        system->setCertificateVerifier(context, verifier);
        test(false);
    }
    catch (const IceSSL::CertificateVerifierTypeException&)
    {
        std::cout << "ok" << std::endl;
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
}

void
testExpectNoException(const IceSSL::SystemPtr& system,
                      IceSSL::ContextType context,
                      const IceSSL::CertificateVerifierPtr& verifier)
{
    try
    {
        system->setCertificateVerifier(context, verifier);
        std::cout << "ok" << std::endl;
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
}

int
run(int argc, char* argv[], const Ice::CommunicatorPtr& communicator)
{
    IceSSL::SystemPtr system = communicator->getSslSystem();

    ::IceSSL::CertificateVerifierPtr badVerifier  = new BadCertificateVerifier();
    ::IceSSL::CertificateVerifierPtr goodVerifier = new GoodCertificateVerifier();

    //
    // Testing IceSSL::Client context type.
    //

    std::cout << "Setting Certificate Verifiers on Client context." << std::endl;

    std::cout << "Setting verifier of wrong type... ";
    testExpectCertificateVerifierTypeException(system, IceSSL::Client, badVerifier);

    std::cout << "Setting verifier of correct type... ";
    testExpectNoException(system, IceSSL::Client, goodVerifier);

    //
    // Testing IceSSL::Server context type.
    //

    std::cout << "Setting Certificate Verifiers on Server context." << std::endl;

    std::cout << "Setting verifier of wrong type... ";
    testExpectCertificateVerifierTypeException(system, IceSSL::Server, badVerifier);

    std::cout << "Setting verifier of correct type... ";
    testExpectNoException(system, IceSSL::Server, goodVerifier);

    //
    // Testing IceSSL::ClientServer context type.
    //

    std::cout << "Setting Certificate Verifiers on Client and Server contexts." << std::endl;

    std::cout << "Setting verifier of wrong type... ";
    testExpectCertificateVerifierTypeException(system, IceSSL::ClientServer, badVerifier);

    std::cout << "Setting verifier of correct type... ";
    testExpectNoException(system, IceSSL::ClientServer, goodVerifier);

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
