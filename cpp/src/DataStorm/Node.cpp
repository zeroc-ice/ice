// Copyright (c) ZeroC, Inc.

#include "DataStorm/Node.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "NodeI.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStorm;
using namespace Ice;

const char*
NodeShutdownException::what() const noexcept
{
    return "::DataStorm::NodeShutdownException";
}

PropertiesPtr
Node::defaultProperties()
{
    auto properties = make_shared<Properties>();

    // A node depends on the requests it receives on a connection being dispatched in the order they were sent: a
    // partial update is applied to the value left by the preceding sample. The server adapter gets that from its
    // own serialized thread pool (see Instance::init), but the sessions carried by a connection the node opened
    // are dispatched on the client thread pool, which the node does not otherwise configure.
    //
    // The client thread pool ignores this setting while it holds a single thread, which is its default, so this
    // matters only for a communicator configured to let the pool grow - the configuration in which the order
    // would otherwise be lost.
    properties->setProperty("Ice.ThreadPool.Client.Serialize", "1");
    return properties;
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
        InitializationData initData;
        initData.properties = defaultProperties();
        communicator = Ice::initialize(std::move(initData)); // the only call that can throw up to here
    }

    try
    {
        _instance = make_shared<DataStormI::Instance>(communicator, std::move(options.customExecutor));
        _instance->init(std::move(options.serverAuthenticationOptions));
    }
    catch (...)
    {
        if (_instance)
        {
            _instance->destroy(_ownsCommunicator);
        }
        else if (_ownsCommunicator)
        {
            communicator->destroy();
        }
        throw;
    }
    _factory = _instance->getTopicFactory();
}

Node::Node(Ice::CommunicatorPtr communicator)
    : Node{NodeOptions{
          .communicator = std::move(communicator),
          .nodeOwnsCommunicator = false,
          .serverAuthenticationOptions = std::nullopt}}
{
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
    if (_instance)
    {
        _instance->shutdown();
    }
}

bool
Node::isShutdown() const noexcept
{
    return _instance ? _instance->isShutdown() : true;
}

void
Node::waitForShutdown() const noexcept
{
    if (_instance)
    {
        _instance->waitForShutdown();
    }
}

Node&
Node::operator=(Node&& node) noexcept
{
    if (this != &node)
    {
        if (_instance)
        {
            _instance->destroy(_ownsCommunicator);
        }
        _instance = std::move(node._instance);
        _factory = std::move(node._factory);
        _ownsCommunicator = node._ownsCommunicator;
    }
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
