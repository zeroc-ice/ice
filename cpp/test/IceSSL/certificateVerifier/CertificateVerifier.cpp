// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <IceSSL/CertificateVerifier.h>
#include <IceSSL/CertificateVerifierOpenSSL.h>
#include <IceSSL/Exception.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Ice;

//
// Certificate Verifier definitions
//

class BadCertificateVerifier : virtual public ::IceSSL::CertificateVerifier
{
public:
    virtual void setContext(IceSSL::ContextType type) { };
};

class GoodCertificateVerifier : virtual public ::IceSSL::CertificateVerifierOpenSSL
{
public:
    virtual int verify(int, X509_STORE_CTX*, SSL*);
    virtual void setContext(IceSSL::ContextType type) { };
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
testExpectCertificateVerifierTypeException(const IceSSL::PluginPtr& plugin,
                                           IceSSL::ContextType context,
                                           const IceSSL::CertificateVerifierPtr& verifier)
{
    try
    {
        plugin->setCertificateVerifier(context, verifier);
        test(false);
    }
    catch(const IceSSL::CertificateVerifierTypeException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch(const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch(...)
    {
        //
        // Unknown exceptions are always bad.
        //

        test(false);
    }
}

void
testExpectNoException(const IceSSL::PluginPtr& plugin,
                      IceSSL::ContextType context,
                      const IceSSL::CertificateVerifierPtr& verifier)
{
    try
    {
        plugin->setCertificateVerifier(context, verifier);
        std::cout << "ok" << std::endl;
    }
    catch(const Ice::LocalException&)
    {
        //
        // Any other exception is bad.
        //

        test(false);
    }
    catch(...)
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
    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    IceSSL::CertificateVerifierPtr badVerifier  = new BadCertificateVerifier();
    IceSSL::CertificateVerifierPtr goodVerifier = new GoodCertificateVerifier();

    //
    // Testing IceSSL::Client context type.
    //

    std::cout << "setting Certificate Verifiers on Client context." << std::endl;

    std::cout << "setting verifier of wrong type... " << std::flush;
    testExpectCertificateVerifierTypeException(sslPlugin, IceSSL::Client, badVerifier);

    std::cout << "setting verifier of correct type... " << std::flush;
    testExpectNoException(sslPlugin, IceSSL::Client, goodVerifier);

    //
    // Testing IceSSL::Server context type.
    //

    std::cout << "setting Certificate Verifiers on Server context." << std::endl;

    std::cout << "setting verifier of wrong type... " << std::flush;
    testExpectCertificateVerifierTypeException(sslPlugin, IceSSL::Server, badVerifier);

    std::cout << "setting verifier of correct type... " << std::flush;
    testExpectNoException(sslPlugin, IceSSL::Server, goodVerifier);

    //
    // Testing IceSSL::ClientServer context type.
    //

    std::cout << "setting Certificate Verifiers on Client and Server contexts." << std::endl;

    std::cout << "setting verifier of wrong type... " << std::flush;
    testExpectCertificateVerifierTypeException(sslPlugin, IceSSL::ClientServer, badVerifier);

    std::cout << "setting verifier of correct type... " << std::flush;
    testExpectNoException(sslPlugin, IceSSL::ClientServer, goodVerifier);

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
