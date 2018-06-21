// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>

#include <TestHelper.h>

using namespace std;
using namespace IceGrid;

namespace
{

class RegistryPluginI : public Ice::Plugin
{
public:

    RegistryPluginI(const Ice::CommunicatorPtr&);

    virtual void initialize();
    virtual void destroy();

private:

    const Ice::CommunicatorPtr _communicator;
    ReplicaGroupFilterPtr _filterByServer;
    ReplicaGroupFilterPtr _excludeServer2;
    ReplicaGroupFilterPtr _excludeServer3;
    TypeFilterPtr _type;
};

class ReplicaGroupFilterI : public IceGrid::ReplicaGroupFilter
{
public:

    ReplicaGroupFilterI(const RegistryPluginFacadePtr& facade) : _facade(facade), _testFacade(true)
    {
    }

    virtual Ice::StringSeq
    filter(const string& id, const Ice::StringSeq& adpts, const Ice::ConnectionPtr&, const Ice::Context& ctx)
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

        Ice::Context::const_iterator p = ctx.find("server");
        if(p == ctx.end())
        {
            return adpts;
        }

        string server = p->second;
        Ice::StringSeq filteredAdapters;
        for(Ice::StringSeq::const_iterator p = adpts.begin(); p != adpts.end(); ++p)
        {
            if(_facade->getAdapterServer(*p) == server)
            {
                filteredAdapters.push_back(*p);
            }
        }
        return filteredAdapters;
    }

private:

    RegistryPluginFacadePtr _facade;
    bool _testFacade;
};

class TypeFilterI : public IceGrid::TypeFilter
{
public:

    TypeFilterI(const RegistryPluginFacadePtr& facade) : _facade(facade)
    {
    }

    virtual Ice::ObjectProxySeq
    filter(const string& type, const Ice::ObjectProxySeq& objects, const Ice::ConnectionPtr&, const Ice::Context& ctx)
    {
        Ice::Context::const_iterator p = ctx.find("server");
        if(p == ctx.end())
        {
            return objects;
        }

        string server = p->second;
        Ice::ObjectProxySeq filteredObjects;
        for(Ice::ObjectProxySeq::const_iterator p = objects.begin(); p != objects.end(); ++p)
        {
            if(_facade->getAdapterServer((*p)->ice_getAdapterId()) == server)
            {
                filteredObjects.push_back(*p);
            }
        }
        return filteredObjects;
    }

private:

    RegistryPluginFacadePtr _facade;
};

class ExcludeReplicaGroupFilterI : public IceGrid::ReplicaGroupFilter
{
public:

    ExcludeReplicaGroupFilterI(const RegistryPluginFacadePtr& facade, const string& exclude) :
        _facade(facade), _exclude(exclude)
    {
    }

    virtual Ice::StringSeq
    filter(const string& id, const Ice::StringSeq& adapters, const Ice::ConnectionPtr& con, const Ice::Context& ctx)
    {
        Ice::Context::const_iterator p = ctx.find("server");
        if(p == ctx.end() || p->second == _exclude)
        {
            return Ice::StringSeq();
        }
        return adapters;
    }

private:

    const RegistryPluginFacadePtr _facade;
    const string _exclude;
};

}

//
extern "C"
{

ICE_DECLSPEC_EXPORT Ice::Plugin*
createRegistryPlugin(const Ice::CommunicatorPtr& communicator, const string&, const Ice::StringSeq&)
{
    return new RegistryPluginI(communicator);
}

}

RegistryPluginI::RegistryPluginI(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
RegistryPluginI::initialize()
{
    IceGrid::RegistryPluginFacadePtr facade = IceGrid::getRegistryPluginFacade();
    assert(facade);

    _filterByServer = new ReplicaGroupFilterI(facade);
    _excludeServer2 = new ExcludeReplicaGroupFilterI(facade, "Server2");
    _excludeServer3 = new ExcludeReplicaGroupFilterI(facade, "Server3");
    _type = new TypeFilterI(facade);

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
    IceGrid::RegistryPluginFacadePtr facade = IceGrid::getRegistryPluginFacade();
    assert(facade);

    facade->removeReplicaGroupFilter("filterByServer", _filterByServer);
    facade->removeReplicaGroupFilter("excludeServer", _excludeServer2);
    facade->removeReplicaGroupFilter("excludeServer", _excludeServer3);
    facade->removeTypeFilter("::Test::TestIntf2", _type);
}
