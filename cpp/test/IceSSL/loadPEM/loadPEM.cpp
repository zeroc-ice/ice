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
#include <Ice/System.h>

using namespace std;
using namespace Ice;

class PEMLoadingClient : public Application
{
public:

    virtual int run(int, char*[]);

    void testExpectCertificateAndPrivateKeyLoadException(const std::string&);
    void testExpectPrivateKeyLoadException(const std::string&);
    void testExpectCertificateLoadException(const std::string&);
    void testExpectCertificateKeyMatchException(const std::string&);
    void testNoException(const std::string&);

private:
    PropertiesPtr _properties;
    IceSSL::SystemPtr _system;
};

int
main(int argc, char* argv[])
{
    PEMLoadingClient app;
    return app.main(argc, argv, "");
}

void
PEMLoadingClient::testExpectCertificateAndPrivateKeyLoadException(const std::string& configFile)
{
    try
    {
        _properties->setProperty("Ice.SSL.Client.Config", configFile);
        _system->configure(IceSSL::Client);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::CertificateLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const IceSSL::OpenSSL::PrivateKeyLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const LocalException& localEx)
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
PEMLoadingClient::testExpectPrivateKeyLoadException(const std::string& configFile)
{
    try
    {
        _properties->setProperty("Ice.SSL.Client.Config", configFile);
        _system->configure(IceSSL::Client);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::PrivateKeyLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const LocalException& localEx)
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
PEMLoadingClient::testExpectCertificateLoadException(const std::string& configFile)
{
    try
    {
        _properties->setProperty("Ice.SSL.Client.Config", configFile);
        _system->configure(IceSSL::Client);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::CertificateLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const LocalException& localEx)
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
PEMLoadingClient::testExpectCertificateKeyMatchException(const std::string& configFile)
{
    try
    {
        _properties->setProperty("Ice.SSL.Client.Config", configFile);
        _system->configure(IceSSL::Client);
        std::cout << "failed" << std::endl;
        abort();
    }
    catch (const IceSSL::OpenSSL::CertificateKeyMatchException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch (const LocalException& localEx)
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
PEMLoadingClient::testNoException(const std::string& configFile)
{
    try
    {
        _properties->setProperty("Ice.SSL.Client.Config", configFile);
        _system->configure(IceSSL::Client);
        std::cout << "ok" << std::endl;
    }
    catch (const LocalException& localEx)
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
PEMLoadingClient::run(int argc, char* argv[])
{
    _properties = communicator()->getProperties();
    _system = communicator()->getSslSystem();

    _properties->setProperty("Ice.SSL.Client.CertPath", "../certs");

    std::cout << "Bad private key and certificate... ";
    testExpectCertificateAndPrivateKeyLoadException("sslconfig_1.xml");

    std::cout << "Bad private key and good certificate 1... ";
    testExpectPrivateKeyLoadException("sslconfig_2.xml");

    std::cout << "Good private key 1 and bad certificate... ";
    testExpectCertificateLoadException("sslconfig_3.xml");

    std::cout << "Good private key 1 and good certificate 2, mismatched... ";
    testExpectCertificateKeyMatchException("sslconfig_4.xml");

    std::cout << "Good private key 2 and good certificate 1, mismatched (again)... ";
    testExpectCertificateKeyMatchException("sslconfig_5.xml");

    std::cout << "Good matched private key and certificate... ";
    testNoException("sslconfig_6.xml");

    return EXIT_SUCCESS;
}
