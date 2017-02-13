// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestCommon.h>

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
