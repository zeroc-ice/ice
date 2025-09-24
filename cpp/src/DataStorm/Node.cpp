// Copyright (c) ZeroC, Inc.

#include "DataStorm/Node.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "NodeI.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStorm;
using namespace Ice;

namespace
{
    CommunicatorPtr createCommunicator()
    {
        PropertiesPtr properties = make_shared<Properties>(vector<string>{"DataStorm"});
        InitializationData initData{.properties = properties};
        return initialize(std::move(initData));
    }

    CommunicatorPtr argsToCommunicator(int& argc, const char* argv[])
    {
        PropertiesPtr properties = createProperties(argc, argv, make_shared<Properties>(vector<string>{"DataStorm"}));
        InitializationData initData{.properties = properties};
        return initialize(std::move(initData));
    }

#ifdef _WIN32
    CommunicatorPtr argsToCommunicator(int& argc, const wchar_t* argv[])
    {
        PropertiesPtr properties = createProperties(argc, argv, make_shared<Properties>(vector<string>{"DataStorm"}));
        InitializationData initData{.properties = properties};
        return initialize(std::move(initData));
    }
#endif
}

const char*
NodeShutdownException::what() const noexcept
{
    return "::DataStorm::NodeShutdownException";
}

Node::Node(CommunicatorPtr communicator, function<void(function<void()> call)> customExecutor)
    : Node(std::move(communicator), std::move(customExecutor), false)
{
}

Node::Node(int& argc, const char* argv[], function<void(function<void()> call)> customExecutor)
    : Node(argsToCommunicator(argc, argv), std::move(customExecutor), true)
{
}

#ifdef _WIN32
Node::Node(int& argc, const wchar_t* argv[], function<void(function<void()> call)> customExecutor)
    : Node(argsToCommunicator(argc, argv), std::move(customExecutor), true)
{
}
#endif

Node::Node(function<void(function<void()> call)> customExecutor)
    : Node(createCommunicator(), std::move(customExecutor), true)
{
}

Node::Node(
    CommunicatorPtr communicator, // NOLINT(performance-unnecessary-value-param)
    std::function<void(std::function<void()> call)> customExecutor,
    bool ownsCommunicator)
    : _ownsCommunicator(ownsCommunicator)
{
    try
    {
        _instance = make_shared<DataStormI::Instance>(communicator, std::move(customExecutor));
        _instance->init();
    }
    catch (...)
    {
        if (_ownsCommunicator)
        {
            communicator->destroy();
        }
        throw;
    }
    _factory = _instance->getTopicFactory();
}

Node::Node(Node&& node) noexcept
{
    _instance = std::move(node._instance);
    _factory = std::move(node._factory);
    _ownsCommunicator = node._ownsCommunicator;
}

Node::~Node()
{
    if (_instance)
    {
        _instance->destroy(_ownsCommunicator);
    }
}

void
Node::shutdown() noexcept
{
    _instance->shutdown();
}

bool
Node::isShutdown() const noexcept
{
    return _instance->isShutdown();
}

void
Node::waitForShutdown() const noexcept
{
    _instance->waitForShutdown();
}

Node&
Node::operator=(Node&& node) noexcept
{
    _instance = std::move(node._instance);
    _factory = std::move(node._factory);
    _ownsCommunicator = node._ownsCommunicator;
    return *this;
}

CommunicatorPtr
Node::getCommunicator() const noexcept
{
    return _instance ? _instance->getCommunicator() : nullptr;
}

ConnectionPtr
Node::getSessionConnection(string_view ident) const noexcept
{
    return _instance ? _instance->getNode()->getSessionConnection(ident) : nullptr;
}
