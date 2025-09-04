// Copyright (c) ZeroC, Inc.

#include "TestI.h"
#include "Ice/Ice.h"

using namespace Ice;
using namespace Test;
using namespace std;

namespace
{
    //
    // A no-op Logger, used when testing the Logger Admin
    //

    class NullLogger final : public Logger, public enable_shared_from_this<NullLogger>
    {
    public:
        void print(const string&) final {}

        void trace(const string&, const string&) final {}

        void warning(const string&) final {}

        void error(const string&) final {}

        string getPrefix() final { return "NullLogger"; }

        LoggerPtr cloneWithPrefix(string) final { return shared_from_this(); }
    };
}

RemoteCommunicatorI::RemoteCommunicatorI(CommunicatorPtr communicator)
    : _communicator(std::move(communicator)),
      _removeCallback(nullptr)
{
}

optional<ObjectPrx>
RemoteCommunicatorI::getAdmin(const Current&)
{
    return _communicator->getAdmin();
}

PropertyDict
RemoteCommunicatorI::getChanges(const Current&)
{
    lock_guard lock(_mutex);

    if (_removeCallback)
    {
        return _changes;
    }
    else
    {
        return {};
    }
}

void
RemoteCommunicatorI::addUpdateCallback()
{
    lock_guard lock(_mutex);

    auto admin = _communicator->findAdminFacet<NativePropertiesAdmin>("Properties");
    if (admin)
    {
        _removeCallback = admin->addUpdateCallback([this](const PropertyDict& changes) { updated(changes); });
    }
}

void
RemoteCommunicatorI::addUpdateCallback(const Current&)
{
    addUpdateCallback();
}

void
RemoteCommunicatorI::removeUpdateCallback(const Current&)
{
    lock_guard lock(_mutex);

    auto admin = _communicator->findAdminFacet<NativePropertiesAdmin>("Properties");
    if (admin)
    {
        if (_removeCallback)
        {
            _removeCallback();
            _removeCallback = nullptr;
        }
    }
}

void
RemoteCommunicatorI::print(string message, const Current&)
{
    _communicator->getLogger()->print(message);
}
void
RemoteCommunicatorI::trace(string category, string message, const Current&)
{
    _communicator->getLogger()->trace(category, message);
}
void
RemoteCommunicatorI::warning(string message, const Current&)
{
    _communicator->getLogger()->warning(message);
}
void
RemoteCommunicatorI::error(string message, const Current&)
{
    _communicator->getLogger()->error(message);
}

void
RemoteCommunicatorI::shutdown(const Current&)
{
    _communicator->shutdown();
}

void
RemoteCommunicatorI::waitForShutdown(const Current&)
{
    //
    // Note that we are executing in a thread of the *main* communicator,
    // not the one that is being shut down.
    //
    _communicator->waitForShutdown();
}

void
RemoteCommunicatorI::destroy(const Current&)
{
    _communicator->destroy();
}

void
RemoteCommunicatorI::updated(const PropertyDict& changes)
{
    lock_guard lock(_mutex);
    _changes = changes;
}

optional<RemoteCommunicatorPrx>
RemoteCommunicatorFactoryI::createCommunicator(PropertyDict props, const Current& current)
{
    //
    // Prepare the property set using the given properties.
    //
    InitializationData init;
    init.properties = createProperties();
    for (const auto& prop : props)
    {
        init.properties->setProperty(prop.first, prop.second);
    }

    if (init.properties->getPropertyAsInt("NullLogger") > 0)
    {
        init.logger = make_shared<NullLogger>();
    }

    //
    // Initialize a new communicator.
    //
    CommunicatorPtr communicator = initialize(init);

    //
    // Install a custom admin facet.
    //
    communicator->addAdminFacet(make_shared<TestFacetI>(), "TestFacet");

    //
    // Set the callback on the admin facet.
    //
    RemoteCommunicatorIPtr servant = make_shared<RemoteCommunicatorI>(communicator);
    servant->addUpdateCallback();

    return current.adapter->addWithUUID<RemoteCommunicatorPrx>(servant);
}

void
RemoteCommunicatorFactoryI::shutdown(const Current& current)
{
    current.adapter->getCommunicator()->shutdown();
}
