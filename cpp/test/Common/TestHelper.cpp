// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <TestHelper.h>
#include <Ice/RegisterPlugins.h>
#include <IceUtil/CtrlCHandler.h>

using namespace std;
using namespace Test;

namespace
{

#if !defined(ICE_OS_UWP) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
Test::TestHelper* instance = 0;

void
shutdownOnInterruptCallback(int)
{
    if(instance)
    {
        instance->shutdown();
    }
}

#endif

}

Test::TestHelper::TestHelper(bool registerPlugins)
#if !defined(ICE_OS_UWP) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
    : _ctrlCHandler(0)
#endif
{
#if !defined(ICE_OS_UWP) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
    instance = this;
#endif

    if(registerPlugins)
    {
#ifdef ICE_STATIC_LIBS
        Ice::registerIceSSL(false);
        Ice::registerIceWS(true);
        Ice::registerIceUDP(true);
#   ifdef ICE_HAS_BT
        Ice::registerIceBT(false);
#   endif
#endif
    }

#if defined(_WIN32) && !defined(ICE_OS_UWP)
    _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_ERROR, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_ERROR, _CRTDBG_FILE_STDERR);

    _CrtSetReportMode(_CRT_WARN, _CRTDBG_MODE_FILE);
    _CrtSetReportFile(_CRT_WARN, _CRTDBG_FILE_STDERR);

    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
#endif
}

Test::TestHelper::~TestHelper()
{
#if !defined(ICE_OS_UWP) && (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
    if(_ctrlCHandler)
    {
        delete _ctrlCHandler;
        _ctrlCHandler = 0;
    }
#endif
}

void
Test::TestHelper::setControllerHelper(ControllerHelper* controllerHelper)
{
    assert(!_controllerHelper);
    _controllerHelper = controllerHelper;
}

string
Test::TestHelper::getTestEndpoint(const std::string& prot)
{
    assert(_communicator);
    return getTestEndpoint(_communicator->getProperties(), 0, prot);
}

string
Test::TestHelper::getTestEndpoint(int num, const std::string& prot)
{
    assert(_communicator);
    return getTestEndpoint(_communicator->getProperties(), num, prot);
}

std::string
Test::TestHelper::getTestEndpoint(const Ice::PropertiesPtr& properties, int num, const std::string& prot)
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
            {
                ostr << "default -u 5e08f4de-5015-4507-abe1-a7807002db3d";
                break;
            }
            case 1:
            {
                ostr << "default -u dae56460-2485-46fd-a3ca-8b730e1e868b";
                break;
            }
            case 2:
            {
                ostr << "default -u 99e08bc6-fcda-4758-afd0-a8c00655c999";
                break;
            }
            default:
            {
                assert(false);
            }
        }
    }
    else
    {
        ostr << protocol << " -p " << (basePort + num);
    }
    return ostr.str();
}

string
Test::TestHelper::getTestHost()
{
    assert(_communicator);
    return getTestHost(_communicator->getProperties());
}

string
Test::TestHelper::getTestHost(const Ice::PropertiesPtr& properties)
{
    return properties->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
}

string
Test::TestHelper::getTestProtocol()
{
    assert(_communicator);
    return getTestProtocol(_communicator->getProperties());
}

string
Test::TestHelper::getTestProtocol(const Ice::PropertiesPtr& properties)
{
    return properties->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
}

int
Test::TestHelper::getTestPort(int num)
{
    assert(_communicator);
    return getTestPort(_communicator->getProperties(), num);
}

int
Test::TestHelper::getTestPort(const Ice::PropertiesPtr& properties, int num)
{
    return properties->getPropertyAsIntWithDefault("Test.BasePort", 12010) + num;
}

Ice::PropertiesPtr
Test::TestHelper::createTestProperties(int& argc, char* argv[])
{
    Ice::PropertiesPtr properties = Ice::createProperties(argc, argv);
    Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
    args = properties->parseCommandLineOptions("Test", args);
    args = properties->parseCommandLineOptions("TestAdapter", args);
    Ice::stringSeqToArgs(args, argc, argv);
    return properties;
}

Ice::CommunicatorPtr
Test::TestHelper::initialize(int& argc, char* argv[], const Ice::PropertiesPtr& properties)
{
    Ice::InitializationData initData;
    initData.properties = properties ? properties : createTestProperties(argc, argv);
    return initialize(argc, argv, initData);
}

Ice::CommunicatorPtr
Test::TestHelper::initialize(int& argc, char* argv[], Ice::InitializationData initData)
{
    _communicator = Ice::initialize(argc, argv, initData);
    return _communicator;
}

Ice::CommunicatorPtr
Test::TestHelper::communicator() const
{
    return _communicator;
}

void
Test::TestHelper::serverReady()
{
#if defined(ICE_OS_UWP) || (TARGET_OS_IPHONE != 0)
    assert(_controllerHelper);
    _controllerHelper->serverReady();
#endif
}

void
Test::TestHelper::shutdown()
{
    if(_communicator)
    {
        _communicator->shutdown();
    }
}

#if defined(ICE_OS_UWP) || (TARGET_OS_IPHONE != 0)
void
Test::TestHelper::shutdownOnInterrupt()
{
}
#else
void
Test::TestHelper::shutdownOnInterrupt()
{
    assert(!_ctrlCHandler);
    if(_ctrlCHandler == 0)
    {
        _ctrlCHandler = new IceUtil::CtrlCHandler();
    }
    _ctrlCHandler->setCallback(shutdownOnInterruptCallback);
}
#endif
