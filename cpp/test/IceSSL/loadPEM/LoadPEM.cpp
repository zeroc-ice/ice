// **********************************************************************
//
// Copyright (c) 2002
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <TestCommon.h>
#include <IceSSL/Exception.h>
#include <IceSSL/Plugin.h>

using namespace std;
using namespace Ice;

void
testExpectCertificateAndPrivateKeyLoadException(const Ice::CommunicatorPtr& communicator,
                                                const std::string& configFile)
{
    PropertiesPtr properties = communicator->getProperties();
    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    try
    {
        properties->setProperty("IceSSL.Client.Config", configFile);
        sslPlugin->configure(IceSSL::Client);
        test(false);
    }
    catch(const IceSSL::CertificateLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch(const IceSSL::PrivateKeyLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch(const RuntimeException&)
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
testExpectPrivateKeyLoadException(const Ice::CommunicatorPtr& communicator, const std::string& configFile)
{
    PropertiesPtr properties = communicator->getProperties();
    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    try
    {
        properties->setProperty("IceSSL.Client.Config", configFile);
        sslPlugin->configure(IceSSL::Client);
        test(false);
    }
    catch(const IceSSL::PrivateKeyLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch(const RuntimeException&)
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
testExpectCertificateLoadException(const Ice::CommunicatorPtr& communicator, const std::string& configFile)
{
    PropertiesPtr properties = communicator->getProperties();
    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    try
    {
        properties->setProperty("IceSSL.Client.Config", configFile);
        sslPlugin->configure(IceSSL::Client);
        test(false);
    }
    catch(const IceSSL::CertificateLoadException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch(const RuntimeException&)
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
testExpectCertificateKeyMatchException(const Ice::CommunicatorPtr& communicator, const std::string& configFile)
{
    PropertiesPtr properties = communicator->getProperties();
    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    try
    {
        properties->setProperty("IceSSL.Client.Config", configFile);
        sslPlugin->configure(IceSSL::Client);
        test(false);
    }
    catch(const IceSSL::CertificateKeyMatchException&)
    {
        std::cout << "ok" << std::endl;
    }
    catch(const RuntimeException&)
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
testNoException(const Ice::CommunicatorPtr& communicator, const std::string& configFile)
{
    PropertiesPtr properties = communicator->getProperties();
    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    try
    {
        properties->setProperty("IceSSL.Client.Config", configFile);
        sslPlugin->configure(IceSSL::Client);
        std::cout << "ok" << std::endl;
    }
    catch(const RuntimeException&)
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
    PropertiesPtr properties = communicator->getProperties();
    // properties->setProperty("IceSSL.Client.CertPath", "../certs");

    std::cout << "bad private key and certificate... " << std::flush;
    testExpectCertificateAndPrivateKeyLoadException(communicator, "sslconfig_1.xml");

    std::cout << "bad private key and good certificate 1... " << std::flush;
    testExpectPrivateKeyLoadException(communicator, "sslconfig_2.xml");

    std::cout << "good private key 1 and bad certificate... " << std::flush;
    testExpectCertificateLoadException(communicator, "sslconfig_3.xml");

    std::cout << "good private key 1 and good certificate 2, mismatched... " << std::flush;
    testExpectCertificateKeyMatchException(communicator, "sslconfig_4.xml");

    std::cout << "good private key 2 and good certificate 1, mismatched (again)... " << std::flush;
    testExpectCertificateKeyMatchException(communicator, "sslconfig_5.xml");

    std::cout << "good matched private key and certificate... " << std::flush;
    testNoException(communicator, "sslconfig_6.xml");

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
