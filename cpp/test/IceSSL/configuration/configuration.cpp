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

// Note: This test must have a valid Ice.SSL.Client.CertPath
//       and Ice.SSL.Server.CertPath specified.

using namespace std;
using namespace Ice;

class ConfigurationClient : public Application
{
public:

    virtual int run(int, char*[]);
    void testContextNoConfig(IceSSL::ContextType);
    void testContextWithConfig(IceSSL::ContextType, const std::string&, const std::string&, bool expectFailure = true);
};

int
main(int argc, char* argv[])
{
    ConfigurationClient app;
    return app.main(argc, argv, "");
}


void
ConfigurationClient::testContextNoConfig(IceSSL::ContextType contextType)
{
    testContextWithConfig(contextType, "", "");
}

void
ConfigurationClient::testContextWithConfig(IceSSL::ContextType contextType,
                                           const std::string& clientFile,
                                           const std::string& serverFile,
                                           bool expectFailure)
{
    PropertiesPtr properties = communicator()->getProperties();
    IceSSL::SystemPtr sslSystem = communicator()->getSslSystem();

    std::string contextString;

    std::string clientPropertyString = "Ice.SSL.Client.Config";
    std::string serverPropertyString = "Ice.SSL.Server.Config";

    switch (contextType)
    {
        case IceSSL::Client:
        {
            contextString = "Client";
            break;
        }

        case IceSSL::Server:
        {
            contextString = "Server";
            break;
        }

        case IceSSL::ClientServer:
        {
            contextString = "ClientServer";
            break;
        }
    }

    std::string configFileDesc = "";

    if (!clientFile.empty() && !serverFile.empty())
    {
        configFileDesc = "client and server configuration files";
    }
    else if (!clientFile.empty())
    {
        configFileDesc = "client configuration file";
    }
    else if (!serverFile.empty())
    {
        configFileDesc = "server configuration file";
    }
    else
    {
        configFileDesc = "no configuration file";
    }

    std::cout << contextString << " with " << configFileDesc << "... ";

    try
    {
        properties->setProperty(clientPropertyString, clientFile);
        properties->setProperty(serverPropertyString, serverFile);
        sslSystem->configure(contextType);

        if (expectFailure)
        {
            std::cout << "failed" << std::endl;
            abort();
        }
        else
        {
            std::cout << "ok" << std::endl;
        }
    }
    catch (const IceSSL::ConfigurationLoadingException& configEx)
    {
        //
        // Depending on the context type, and if we supplied
        // a configuration file, this might be a valid response.
        //

        switch (contextType)
        {
            case IceSSL::Client:
            {
                if (clientFile.empty())
                {
                    std::cout << "ok" << std::endl;
                }
                else
                {
                    std::cout << "failed" << std::endl;
                    std::cout << configEx << std::endl;
                    abort();
                }
                break;
            }

            case IceSSL::Server:
            {
                if (serverFile.empty())
                {
                    std::cout << "ok" << std::endl;
                }
                else
                {
                    std::cout << "failed" << std::endl;
                    std::cout << configEx << std::endl;
                    abort();
                }
                break;
            }

            case IceSSL::ClientServer:
            {
                if (clientFile.empty() || serverFile.empty())
                {
                    std::cout << "ok" << std::endl;
                }
                else
                {
                    std::cout << "failed" << std::endl;
                    std::cout << configEx << std::endl;
                    abort();
                }
                break;
            }
        }
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
ConfigurationClient::run(int argc, char* argv[])
{
    // Testing Client context.
    testContextNoConfig(IceSSL::Client);
    testContextWithConfig(IceSSL::Client,"client_sslconfig.xml","");

    // Testing Server context.
    testContextNoConfig(IceSSL::Server);
    testContextWithConfig(IceSSL::Server,"","server_sslconfig.xml");

    // Testing ClientServer context.
    testContextNoConfig(IceSSL::ClientServer);
    testContextWithConfig(IceSSL::ClientServer, "client_sslconfig.xml", "");
    testContextWithConfig(IceSSL::ClientServer, "", "server_sslconfig.xml");
    testContextWithConfig(IceSSL::ClientServer, "client_sslconfig.xml", "server_sslconfig.xml");
    testContextWithConfig(IceSSL::ClientServer, "sslconfig.xml", "sslconfig.xml", false);

    return EXIT_SUCCESS;
}
