// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestCommon.h>
#include <Controller.h>

#include <Ice/Communicator.h>

std::string
getTestEndpoint(const Ice::CommunicatorPtr& communicator, int num, const std::string& protocol)
{
    return getTestEndpoint(communicator->getProperties(), num, protocol);
}

std::string
getTestEndpoint(const Ice::PropertiesPtr& properties, int num, const std::string& prot)
{
    std::ostringstream ostr;
    std::string protocol = prot;
    if(protocol.empty())
    {
        protocol = properties->getPropertyWithDefault("Ice.Default.Protocol", "default");
    }

    int basePort = properties->getPropertyAsIntWithDefault("Test.BasePort", 12010);

    if(protocol == "bt")
    {
        //
        // For Bluetooth, there's no need to specify a port (channel) number.
        // The client locates the server using its address and a UUID.
        //
        switch(num)
        {
        case 0:
            ostr << "default -u 5e08f4de-5015-4507-abe1-a7807002db3d";
            break;
        case 1:
            ostr << "default -u dae56460-2485-46fd-a3ca-8b730e1e868b";
            break;
        case 2:
            ostr << "default -u 99e08bc6-fcda-4758-afd0-a8c00655c999";
            break;
        default:
            assert(false);
        }
    }
    else
    {
        ostr << protocol << " -p " << (basePort + num);
    }
    return ostr.str();
}

std::string
getTestHost(const Ice::PropertiesPtr& properties)
{
    return properties->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
}

std::string
getTestProtocol(const Ice::PropertiesPtr& properties)
{
    return properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
}

int
getTestPort(const Ice::PropertiesPtr& properties, int num)
{
    return properties->getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
}

Ice::InitializationData
getTestInitData(int& argc, char* argv[])
{
    Ice::InitializationData initData;
    initData.properties = Ice::createProperties(argc, argv);
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = initData.properties->parseCommandLineOptions("Test", args);
    Ice::stringSeqToArgs(args, argc, argv);
    return initData;
}

RemoteConfig::RemoteConfig(const std::string& name, int argc, char** argv, const Ice::CommunicatorPtr& communicator) :
    _status(1)
{
    //
    // If ControllerHost is defined, we are using a server on a remote host. We expect a
    // test controller will already be active. We let exceptions propagate out to
    // the caller.
    //
    // Also look for a ConfigName property, which specifies the name of the configuration
    // we are currently testing.
    //
    std::string controllerHost;
    std::string configName;
    for(int i = 1; i < argc; ++i)
    {
        std::string opt = argv[i];
        if(opt.find("--ControllerHost") == 0)
        {
            std::string::size_type pos = opt.find('=');
            if(pos != std::string::npos && opt.size() > pos + 1)
            {
                controllerHost = opt.substr(pos + 1);
            }
        }
        else if(opt.find("--ConfigName") == 0)
        {
            std::string::size_type pos = opt.find('=');
            if(pos != std::string::npos && opt.size() > pos + 1)
            {
                configName = opt.substr(pos + 1);
            }
        }
    }

    Test::Common::ServerPrxPtr server;

    if(!controllerHost.empty())
    {
        std::string prot = communicator->getProperties()->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        std::string host;
        if(prot != "bt")
        {
            host = communicator->getProperties()->getProperty("Ice.Default.Host");
        }

        Test::Common::StringSeq options;

        Test::Common::ControllerPrxPtr controller = ICE_CHECKED_CAST(Test::Common::ControllerPrx,
            communicator->stringToProxy("controller:tcp -h " + controllerHost + " -p 15000"));
        server = controller->runServer("cpp", name, prot, host, false, configName, options);
        server->waitForServer();
    }

    _server = server;
}

RemoteConfig::~RemoteConfig() ICE_NOEXCEPT_FALSE
{
    if(_server)
    {
        try
        {
            Test::Common::ServerPrxPtr server = ICE_UNCHECKED_CAST(Test::Common::ServerPrx, _server);
            if(_status == 0)
            {
                server->waitTestSuccess();
            }
            else
            {
                server->terminate();
            }
        }
        catch(const Ice::LocalException&)
        {
            if(_status == 0)
            {
                throw;
            }
        }
    }
}

bool
RemoteConfig::isRemote() const
{
    return _server != 0;
}

void
RemoteConfig::finished(int status)
{
    _status = status;
}
