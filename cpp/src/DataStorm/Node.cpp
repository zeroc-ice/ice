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
        InitializationData initData{.properties = make_shared<Properties>(vector<string>{"DataStorm"})};
        return initialize(std::move(initData));
    }

    NodeOptions createNodeOptions(int& argc, const char* argv[])
    {
        NodeOptions options;
        auto properties = Ice::createProperties(argc, argv, make_shared<Properties>(vector<string>{"DataStorm"}));
        InitializationData initData{.properties = properties};
        options.communicator = Ice::initialize(std::move(initData));
        options.nodeOwnsCommunicator = true;
        return options;
    }

#ifdef _WIN32
    NodeOptions createNodeOptions(int& argc, const wchar_t* argv[])
    {
        NodeOptions options;
        auto properties = Ice::createProperties(argc, argv, make_shared<Properties>(vector<string>{"DataStorm"}));
        InitializationData initData{.properties = properties};
        options.communicator = Ice::initialize(std::move(initData));
        options.nodeOwnsCommunicator = true;
        return options;
    }
#endif
}

const char*
NodeShutdownException::what() const noexcept
{
    return "::DataStorm::NodeShutdownException";
}

Node::Node(NodeOptions options)
{
    auto communicator = options.communicator;
    if (communicator)
    {
        _ownsCommunicator = options.nodeOwnsCommunicator;
    }
    else
    {
        _ownsCommunicator = true;
        communicator = createCommunicator(); // the only call that can throw up to here
    }

    try
    {
        _instance = make_shared<DataStormI::Instance>(communicator, std::move(options.customExecutor));
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

Node::Node(Ice::CommunicatorPtr communicator)
    : Node{NodeOptions{.communicator = std::move(communicator), .nodeOwnsCommunicator = false}}
{
}

Node::Node(int& argc, const char* argv[]) : Node{createNodeOptions(argc, argv)} {}

#ifdef _WIN32
Node::Node(int& argc, const wchar_t* argv[]) : Node{createNodeOptions(argc, argv)} {}
#endif

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
