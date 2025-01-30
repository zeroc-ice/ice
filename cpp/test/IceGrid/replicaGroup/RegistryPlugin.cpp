// Copyright (c) ZeroC, Inc.

#include "Ice/Ice.h"
#include "IceGrid/IceGrid.h"
#include "TestHelper.h"

using namespace std;
using namespace IceGrid;

namespace
{
    class RegistryPluginI final : public Ice::Plugin
    {
    public:
        RegistryPluginI(const shared_ptr<Ice::Communicator>&);

        void initialize() override;
        void destroy() override;

    private:
        const shared_ptr<Ice::Communicator> _communicator;
        shared_ptr<ReplicaGroupFilter> _filterByServer;
        shared_ptr<ReplicaGroupFilter> _excludeServer2;
        shared_ptr<ReplicaGroupFilter> _excludeServer3;
        shared_ptr<TypeFilter> _type;
    };

    class ReplicaGroupFilterI final : public IceGrid::ReplicaGroupFilter
    {
    public:
        ReplicaGroupFilterI(const shared_ptr<RegistryPluginFacade>& facade) : _facade(facade) {}

        Ice::StringSeq filter(
            const string& id,
            const Ice::StringSeq& adpts,
            const shared_ptr<Ice::Connection>&,
            const Ice::Context& ctx) override
        {
            if (_testFacade)
            {
                _testFacade = false; // Only test once.
                for (const auto& adpt : adpts)
                {
                    try
                    {
                        test(
                            _facade->getApplicationInfo(_facade->getAdapterApplication(adpt)).descriptor.name ==
                            "Test");
                        test(_facade->getServerInfo(_facade->getAdapterServer(adpt)).application == "Test");
                        test(_facade->getNodeInfo(_facade->getAdapterNode(adpt)).name == "localnode");
                        test(_facade->getNodeLoad(_facade->getAdapterNode(adpt)).avg1 >= 0.0);
                        test(_facade->getAdapterInfo(adpt)[0].replicaGroupId == id);
                        test(_facade->getPropertyForAdapter(adpt, "Identity") == id);
                    }
                    catch (const Ice::Exception& ex)
                    {
                        cerr << ex << endl;
                        test(false);
                    }
                }
            }

            auto p = ctx.find("server");
            if (p == ctx.end())
            {
                return adpts;
            }

            string server = p->second;
            Ice::StringSeq filteredAdapters;
            for (const auto& adapter : adpts)
            {
                if (_facade->getAdapterServer(adapter) == server)
                {
                    filteredAdapters.push_back(adapter);
                }
            }
            return filteredAdapters;
        }

    private:
        shared_ptr<RegistryPluginFacade> _facade;
        bool _testFacade{true};
    };

    class TypeFilterI final : public IceGrid::TypeFilter
    {
    public:
        TypeFilterI(const shared_ptr<RegistryPluginFacade>& facade) : _facade(facade) {}

        Ice::ObjectProxySeq filter(
            const string&,
            const Ice::ObjectProxySeq& objects,
            const shared_ptr<Ice::Connection>&,
            const Ice::Context& ctx) override
        {
            auto p = ctx.find("server");
            if (p == ctx.end())
            {
                return objects;
            }

            string server = p->second;
            Ice::ObjectProxySeq filteredObjects;
            for (const auto& object : objects)
            {
                if (_facade->getAdapterServer(object->ice_getAdapterId()) == server)
                {
                    filteredObjects.push_back(object);
                }
            }
            return filteredObjects;
        }

    private:
        shared_ptr<RegistryPluginFacade> _facade;
    };

    class ExcludeReplicaGroupFilterI final : public IceGrid::ReplicaGroupFilter
    {
    public:
        ExcludeReplicaGroupFilterI(const shared_ptr<RegistryPluginFacade>& facade, string exclude)
            : _facade(facade),
              _exclude(std::move(exclude))
        {
        }

        Ice::StringSeq filter(
            const string&,
            const Ice::StringSeq& adapters,
            const shared_ptr<Ice::Connection>&,
            const Ice::Context& ctx) override
        {
            auto p = ctx.find("server");
            if (p == ctx.end() || p->second == _exclude)
            {
                return {};
            }
            return adapters;
        }

    private:
        const shared_ptr<RegistryPluginFacade> _facade;
        const string _exclude;
    };
}

//
extern "C"
{
    ICE_DECLSPEC_EXPORT Ice::Plugin*
    createRegistryPlugin(const shared_ptr<Ice::Communicator>& communicator, const string&, const Ice::StringSeq&)
    {
        return new RegistryPluginI(communicator);
    }
}

RegistryPluginI::RegistryPluginI(const shared_ptr<Ice::Communicator>& communicator) : _communicator(communicator) {}

void
RegistryPluginI::initialize()
{
    auto facade = IceGrid::getRegistryPluginFacade();
    assert(facade);

    _filterByServer = make_shared<ReplicaGroupFilterI>(facade);
    _excludeServer2 = make_shared<ExcludeReplicaGroupFilterI>(facade, "Server2");
    _excludeServer3 = make_shared<ExcludeReplicaGroupFilterI>(facade, "Server3");
    _type = make_shared<TypeFilterI>(facade);

    facade->addReplicaGroupFilter("filterByServer", _filterByServer);
    test(facade->removeReplicaGroupFilter("filterByServer", _filterByServer));
    test(!facade->removeReplicaGroupFilter("filterByServer", _filterByServer));

    facade->addReplicaGroupFilter("filterByServer", _filterByServer);
    facade->addReplicaGroupFilter("excludeServer", _excludeServer2);
    facade->addReplicaGroupFilter("excludeServer", _excludeServer3);
    facade->addTypeFilter("::Test::TestIntf2", _type);
}

void
RegistryPluginI::destroy()
{
    auto facade = IceGrid::getRegistryPluginFacade();
    assert(facade);

    facade->removeReplicaGroupFilter("filterByServer", _filterByServer);
    facade->removeReplicaGroupFilter("excludeServer", _excludeServer2);
    facade->removeReplicaGroupFilter("excludeServer", _excludeServer3);
    facade->removeTypeFilter("::Test::TestIntf2", _type);
}
