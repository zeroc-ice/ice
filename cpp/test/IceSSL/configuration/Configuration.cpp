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
#include <IceSSL/Exception.h>
#include <IceSSL/Plugin.h>

// Note: This test must have a valid IceSSL.Client.CertPath
//       and IceSSL.Server.CertPath specified.

using namespace std;
using namespace Ice;

void testContextWithConfig(const Ice::CommunicatorPtr&, IceSSL::ContextType, const std::string&,
                           const std::string&, bool expectFailure = true);

void
testContextNoConfig(const Ice::CommunicatorPtr& communicator, IceSSL::ContextType contextType)
{
    testContextWithConfig(communicator, contextType, "", "");
}

void
testContextWithConfig(const Ice::CommunicatorPtr& communicator,
                      IceSSL::ContextType contextType,
                      const std::string& clientFile,
                      const std::string& serverFile,
                      bool expectFailure)
{
    PropertiesPtr properties = communicator->getProperties();

    Ice::PluginPtr plugin = communicator->getPluginManager()->getPlugin("IceSSL");
    IceSSL::PluginPtr sslPlugin = IceSSL::PluginPtr::dynamicCast(plugin);

    std::string contextString;

    std::string clientPropertyString = "IceSSL.Client.Config";
    std::string serverPropertyString = "IceSSL.Server.Config";

    switch(contextType)
    {
        case IceSSL::Client:
        {
            contextString = "client";
            break;
        }

        case IceSSL::Server:
        {
            contextString = "server";
            break;
        }

        case IceSSL::ClientServer:
        {
            contextString = "client/server";
            break;
        }
    }

    std::string configFileDesc = "";

    if(!clientFile.empty() && !serverFile.empty())
    {
        configFileDesc = "client and server configuration files";
    }
    else if(!clientFile.empty())
    {
        configFileDesc = "client configuration file";
    }
    else if(!serverFile.empty())
    {
        configFileDesc = "server configuration file";
    }
    else
    {
        configFileDesc = "no configuration file";
    }

    std::cout << contextString << " with " << configFileDesc << "... " << std::flush;

    try
    {
        properties->setProperty(clientPropertyString, clientFile);
        properties->setProperty(serverPropertyString, serverFile);
        sslPlugin->configure(contextType);

        if(expectFailure)
        {
            test(false);
        }
        else
        {
            std::cout << "ok" << std::endl;
        }
    }
    catch(const IceSSL::ConfigurationLoadingException&)
    {
        //
        // Depending on the context type, and if we supplied
        // a configuration file, this might be a valid response.
        //

        switch(contextType)
        {
            case IceSSL::Client:
            {
                if(clientFile.empty())
                {
                    std::cout << "ok" << std::endl;
                }
                else
                {
                    test(false);
                }
                break;
            }

            case IceSSL::Server:
            {
                if(serverFile.empty())
                {
                    std::cout << "ok" << std::endl;
                }
                else
                {
                    test(false);
                }
                break;
            }

            case IceSSL::ClientServer:
            {
                if(clientFile.empty() || serverFile.empty())
                {
                    std::cout << "ok" << std::endl;
                }
                else
                {
                    test(false);
                }
                break;
            }
        }
    }
    catch(const LocalException&)
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
    // Testing Client context.
    testContextNoConfig(communicator, IceSSL::Client);
    testContextWithConfig(communicator, IceSSL::Client,"client_sslconfig.xml","", false);

    // Testing Server context.
    testContextNoConfig(communicator, IceSSL::Server);
    testContextWithConfig(communicator, IceSSL::Server,"","server_sslconfig.xml", false);

    // Testing ClientServer context.
    testContextNoConfig(communicator, IceSSL::ClientServer);
    testContextWithConfig(communicator, IceSSL::ClientServer, "client_sslconfig.xml", "");
    testContextWithConfig(communicator, IceSSL::ClientServer, "", "server_sslconfig.xml");
    testContextWithConfig(communicator, IceSSL::ClientServer, "client_sslconfig.xml", "server_sslconfig.xml", false);
    testContextWithConfig(communicator, IceSSL::ClientServer, "sslconfig.xml", "sslconfig.xml", false);

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
