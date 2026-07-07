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
        communicator = Ice::initialize(); // the only call that can throw up to here
    }

    try
    {
        _instance = make_shared<DataStormI::Instance>(
            communicator,
            std::move(options.customExecutor),
            std::move(options.serverAuthenticationOptions));
        _instance->init();
    }
    catch (...)
    {
        if (_instance)
        {
            // Destroy the instance: this joins the timer and callback executor threads started by the Instance
            // constructor (destroying them still running would call std::terminate) and takes care of the
            // communicator and adapter teardown for both ownership modes.
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
    // A moved-from node is inert and can never transition to shutdown; report it as already shut down.
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
