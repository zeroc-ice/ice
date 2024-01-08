//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>

#include <TestHelper.h>

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

    ReplicaGroupFilterI(const shared_ptr<RegistryPluginFacade>& facade) : _facade(facade), _testFacade(true)
    {
    }

    Ice::StringSeq
    filter(const string& id, const Ice::StringSeq& adpts, const shared_ptr<Ice::Connection>&, const Ice::Context& ctx) override
    {
        if(_testFacade)
        {
            _testFacade = false; // Only test once.
            for(Ice::StringSeq::const_iterator p = adpts.begin(); p != adpts.end(); ++p)
            {
                try
                {
                    test(_facade->getApplicationInfo(_facade->getAdapterApplication(*p)).descriptor.name == "Test");
                    test(_facade->getServerInfo(_facade->getAdapterServer(*p)).application == "Test");
                    test(_facade->getNodeInfo(_facade->getAdapterNode(*p)).name == "localnode");
#ifndef _AIX
                    // On AIX, icegridnode needs read permissions on /dev/kmem
                    test(_facade->getNodeLoad(_facade->getAdapterNode(*p)).avg1 >= 0.0);
#endif
                    test(_facade->getAdapterInfo(*p)[0].replicaGroupId == id);
                    test(_facade->getPropertyForAdapter(*p, "Identity") == id);
                }
                catch(const Ice::Exception& ex)
                {
                    cerr << ex << endl;
                    test(false);
                }
            }
        }

        auto p = ctx.find("server");
        if(p == ctx.end())
        {
            return adpts;
        }

        string server = p->second;
        Ice::StringSeq filteredAdapters;
        for(const auto& adapter : adpts)
        {
            if(_facade->getAdapterServer(adapter) == server)
            {
                filteredAdapters.push_back(adapter);
            }
        }
        return filteredAdapters;
    }

private:

    shared_ptr<RegistryPluginFacade> _facade;
    bool _testFacade;
};

class TypeFilterI final : public IceGrid::TypeFilter
{
public:

    TypeFilterI(const shared_ptr<RegistryPluginFacade>& facade) : _facade(facade)
    {
    }

    Ice::ObjectProxySeq
    filter(const string&, const Ice::ObjectProxySeq& objects, const shared_ptr<Ice::Connection>&,
           const Ice::Context& ctx) override
    {
        auto p = ctx.find("server");
        if(p == ctx.end())
        {
            return objects;
        }

        string server = p->second;
        Ice::ObjectProxySeq filteredObjects;
        for(const auto& object : objects)
        {
            if(_facade->getAdapterServer(object->ice_getAdapterId()) == server)
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

    ExcludeReplicaGroupFilterI(const shared_ptr<RegistryPluginFacade>& facade, const string& exclude) :
        _facade(facade), _exclude(exclude)
    {
    }

    Ice::StringSeq
    filter(const string&, const Ice::StringSeq& adapters, const shared_ptr<Ice::Connection>& ,
           const Ice::Context& ctx) override
    {
        auto p = ctx.find("server");
        if(p == ctx.end() || p->second == _exclude)
        {
            return Ice::StringSeq();
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

RegistryPluginI::RegistryPluginI(const shared_ptr<Ice::Communicator>& communicator) : _communicator(communicator)
{
}

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
