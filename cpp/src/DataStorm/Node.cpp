//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "DataStorm/Node.h"
#include "Ice/Ice.h"
#include "Instance.h"
#include "NodeI.h"
#include "TopicFactoryI.h"

using namespace std;
using namespace DataStorm;

const char*
NodeShutdownException::what() const noexcept
{
    return "::DataStorm::NodeShutdownException";
}

Node::Node(Ice::CommunicatorPtr communicator, function<void(function<void()> call)> customExecutor)
    : _ownsCommunicator(false)
{
    init(communicator, std::move(customExecutor));
}

void
Node::init(const Ice::CommunicatorPtr& communicator, std::function<void(std::function<void()> call)> customExecutor)
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

Ice::CommunicatorPtr
Node::getCommunicator() const noexcept
{
    return _instance ? _instance->getCommunicator() : nullptr;
}

Ice::ConnectionPtr
Node::getSessionConnection(const string& ident) const noexcept
{
    return _instance ? _instance->getNode()->getSessionConnection(ident) : nullptr;
}
