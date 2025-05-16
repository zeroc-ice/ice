// Copyright (c) ZeroC, Inc.

#include "TestHelper.h"
#include "Ice/Ice.h"

#include <algorithm>
#include <sstream>

using namespace std;
using namespace Test;

namespace
{
#if (!defined(__APPLE__) || TARGET_OS_IPHONE == 0)
    Test::TestHelper* instance = nullptr;

    void shutdownOnInterruptCallback(int)
    {
        if (instance)
        {
            instance->shutdown();
        }
    }

#endif

    void parseTestOptions(int& argc, char* argv[], const Ice::PropertiesPtr& properties)
    {
        Ice::StringSeq args = Ice::argsToStringSeq(argc, argv);
        args = properties->parseCommandLineOptions("Test", args);
        args = properties->parseCommandLineOptions("TestAdapter", args);
        Ice::stringSeqToArgs(args, argc, argv);
    }
}

#if TARGET_OS_IPHONE != 0

StreamHelper::StreamHelper() : _controllerHelper(0)
{
    setp(&data[0], &data[sizeof(data) - 1]);

    _previousCoutBuffer = std::cout.rdbuf();
    std::cout.rdbuf(this);

    _previousCerrBuffer = std::cerr.rdbuf();
    std::cerr.rdbuf(this);
}

StreamHelper::~StreamHelper()
{
    std::cout.rdbuf(_previousCoutBuffer);
    std::cerr.rdbuf(_previousCerrBuffer);
}

void
StreamHelper::setControllerHelper(ControllerHelper* controllerHelper)
{
    lock_guard lock(_mutex);
    assert(_controllerHelper && !controllerHelper || !_controllerHelper && controllerHelper);
    _controllerHelper = controllerHelper;

    if (_controllerHelper)
    {
        _previousLogger = Ice::getProcessLogger();
        Ice::setProcessLogger(Ice::getProcessLogger()->cloneWithPrefix(_controllerHelper->loggerPrefix()));
    }
    else
    {
        Ice::setProcessLogger(_previousLogger);
    }
}

void
StreamHelper::flush()
{
}

void
StreamHelper::newLine()
{
    lock_guard lock(_mutex);
    if (_controllerHelper)
    {
        _controllerHelper->print("\n");
    }
}

int
StreamHelper::sync()
{
    std::streamsize n = pptr() - pbase();
    {
        lock_guard lock(_mutex);
        if (_controllerHelper)
        {
            _controllerHelper->print(std::string(pbase(), static_cast<size_t>(n)));
        }
    }
    pbump(-static_cast<int>(pptr() - pbase()));
    return 0;
}

int
StreamHelper::overflow(int ch)
{
    sync();
    if (ch != EOF)
    {
        assert(pptr() != epptr());
        sputc(static_cast<char>(ch));
    }
    return 0;
}

int
StreamHelper::sputc(char c)
{
    if (c == '\n')
    {
        pubsync();
    }
    return std::streambuf::sputc(c);
}

#endif

Test::TestHelper::TestHelper(bool registerPlugins) : _registerPlugins(registerPlugins)
{
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
    instance = this;
#endif

#if defined(_WIN32)
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
#if !defined(__APPLE__) || TARGET_OS_IPHONE == 0
    if (_ctrlCHandler)
    {
        delete _ctrlCHandler;
        _ctrlCHandler = nullptr;
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
Test::TestHelper::getTestEndpoint(const std::string& prot) const
{
    assert(_communicator);
    return getTestEndpoint(_communicator->getProperties(), 0, prot);
}

string
Test::TestHelper::getTestEndpoint(int num, const std::string& prot) const
{
    assert(_communicator);
    return getTestEndpoint(_communicator->getProperties(), num, prot);
}

std::string
Test::TestHelper::getTestEndpoint(const Ice::PropertiesPtr& properties, int num, const std::string& prot)
{
    std::ostringstream ostr;
    std::string protocol = prot;
    if (protocol.empty())
    {
        protocol = properties->getIceProperty("Ice.Default.Protocol");
    }

    int basePort = properties->getPropertyAsIntWithDefault("Test.BasePort", 12010);

    if (protocol == "bt")
    {
        //
        // For Bluetooth, there's no need to specify a port (channel) number.
        // The client locates the server using its address and a UUID.
        //
        switch (num)
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
Test::TestHelper::getTestHost() const
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
Test::TestHelper::getTestProtocol() const
{
    assert(_communicator);
    return getTestProtocol(_communicator->getProperties());
}

string
Test::TestHelper::getTestProtocol(const Ice::PropertiesPtr& properties)
{
    return properties->getIceProperty("Ice.Default.Protocol");
}

int
Test::TestHelper::getTestPort(int num) const
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
    parseTestOptions(argc, argv, properties);
    return properties;
}

Ice::CommunicatorPtr
Test::TestHelper::initialize(Ice::InitializationData initData)
{
    int argc = 0;
    char* argv[] = {nullptr};
    return initialize(argc, argv, std::move(initData));
}

Ice::CommunicatorPtr
Test::TestHelper::initialize(int& argc, char* argv[], const Ice::PropertiesPtr& properties)
{
    Ice::InitializationData initData;
    if (properties)
    {
        parseTestOptions(argc, argv, properties);
        initData.properties = properties;
    }
    else
    {
        initData.properties = createTestProperties(argc, argv);
    }
    return initialize(argc, argv, initData);
}

Ice::CommunicatorPtr
Test::TestHelper::initialize(int& argc, char* argv[], Ice::InitializationData initData)
{
    if (_registerPlugins && IceInternal::isMinBuild())
    {
        auto& factories = initData.pluginFactories;

        Ice::PluginFactory udpPluginFactory{Ice::udpPluginFactory()};
        Ice::PluginFactory wsPluginFactory{Ice::wsPluginFactory()};

        if (none_of(
                factories.begin(),
                factories.end(),
                [&wsPluginFactory](const Ice::PluginFactory& factory)
                { return factory.pluginName == wsPluginFactory.pluginName; }))
        {
            factories.insert(factories.begin(), wsPluginFactory);
        }

        if (none_of(
                factories.begin(),
                factories.end(),
                [&udpPluginFactory](const Ice::PluginFactory& factory)
                { return factory.pluginName == udpPluginFactory.pluginName; }))
        {
            factories.insert(factories.begin(), udpPluginFactory);
        }
    }

    _communicator = Ice::initialize(argc, argv, std::move(initData));
    if (_controllerHelper)
    {
        _controllerHelper->communicatorInitialized(_communicator);
    }
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
    if (_controllerHelper)
    {
        _controllerHelper->serverReady();
    }
}

void
Test::TestHelper::shutdown()
{
    if (_communicator)
    {
        _communicator->shutdown();
    }
}

#if TARGET_OS_IPHONE != 0
void
Test::TestHelper::shutdownOnInterrupt()
{
}
#else
void
Test::TestHelper::shutdownOnInterrupt()
{
    assert(!_ctrlCHandler);
    if (_ctrlCHandler == nullptr)
    {
        _ctrlCHandler = new Ice::CtrlCHandler();
    }
    _ctrlCHandler->setCallback(shutdownOnInterruptCallback);
}
#endif

void
Ice::installTransport(InitializationData& initData)
{
    if (IceInternal::isMinBuild())
    {
        assert(initData.properties);
        string transport = TestHelper::getTestProtocol(initData.properties);
        if (transport == "ws" || transport == "wss")
        {
            auto& factories = initData.pluginFactories;

            Ice::PluginFactory wsPluginFactory{Ice::wsPluginFactory()};

            if (none_of(
                    factories.begin(),
                    factories.end(),
                    [&wsPluginFactory](const Ice::PluginFactory& factory)
                    { return factory.pluginName == wsPluginFactory.pluginName; }))
            {
                factories.insert(factories.begin(), wsPluginFactory);
            }
        }
    }
}
