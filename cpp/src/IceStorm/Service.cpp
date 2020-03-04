//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#define ICESTORM_SERVICE_API_EXPORTS

#include <Ice/PluginManagerI.h> // For loadPlugin

#include <IceStorm/TopicI.h>
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TransientTopicManagerI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Util.h>

#include <IceStorm/Service.h>

#include <IceStorm/Observers.h>
#include <IceStorm/TraceLevels.h>
#include <IceUtil/StringUtil.h>

#include <IceStorm/NodeI.h>
#include <IceStorm/TransientTopicI.h>

#include <IceGrid/Registry.h>

using namespace std;
using namespace Ice;
using namespace IceStorm;
using namespace IceStormInternal;
using namespace IceStormElection;

namespace
{

class ServiceI final : public IceStormInternal::Service
{
public:

    void start(const std::shared_ptr<Ice::Communicator>&,
               const std::shared_ptr<Ice::ObjectAdapter>&,
               const std::shared_ptr<Ice::ObjectAdapter>&,
               const std::string&,
               const Ice::Identity&,
               const std::string&);

    std::shared_ptr<IceStorm::TopicManagerPrx> getTopicManager() const override;

    void start(const std::string&, const std::shared_ptr<Ice::Communicator>&, const Ice::StringSeq&) override;
    void stop() override;

private:

    void createDbEnv(const std::shared_ptr<Ice::Communicator>&);
    void validateProperties(const std::string&,
                            const std::shared_ptr<Ice::Properties>&,
                            const std::shared_ptr<Ice::Logger>&);

    std::shared_ptr<IceStorm::TopicManagerImpl> _manager;
    std::shared_ptr<IceStorm::TransientTopicManagerImpl> _transientManager;
    std::shared_ptr<IceStorm::TopicManagerPrx> _managerProxy;
    std::shared_ptr<IceStorm::Instance> _instance;
};

class FinderI final : public IceStorm::Finder
{
public:

    FinderI(shared_ptr<TopicManagerPrx> topicManager) : _topicManager(move(topicManager))
    {
    }

    shared_ptr<TopicManagerPrx>
    getTopicManager(const Ice::Current&) override
    {
        return _topicManager;
    }

private:

    const shared_ptr<TopicManagerPrx> _topicManager;
};

}

extern "C"
{

ICESTORM_SERVICE_API ::IceBox::Service*
createIceStorm(const shared_ptr<Communicator>&)
{
    return new ServiceI;
}

}

shared_ptr<IceStormInternal::Service>
IceStormInternal::Service::create(const shared_ptr<Communicator>& communicator,
                                  const shared_ptr<ObjectAdapter>& topicAdapter,
                                  const shared_ptr<ObjectAdapter>& publishAdapter,
                                  const string& name,
                                  const Ice::Identity& id,
                                  const string& dbEnv)
{
    shared_ptr<ServiceI> service(new ServiceI);
    service->start(communicator, topicAdapter, publishAdapter, name, id, dbEnv);
    return service;
}

void
ServiceI::start(const string& name, const shared_ptr<Communicator>& communicator, const StringSeq&)
{
    auto properties = communicator->getProperties();

    validateProperties(name, properties, communicator->getLogger());

    int id = properties->getPropertyAsIntWithDefault(name + ".NodeId", -1);

    // If we are using a replicated deployment and if the topic
    // manager thread pool max size is not set then ensure it is set
    // to some suitably high number. This ensures no deadlocks in the
    // replicated case due to call forwarding from replicas to
    // coordinators.
    if(id != -1 && properties->getProperty(name + ".TopicManager.ThreadPool.SizeMax").empty())
    {
        properties->setProperty(name + ".TopicManager.ThreadPool.SizeMax", "100");
    }

    auto topicAdapter = communicator->createObjectAdapter(name + ".TopicManager");
    auto publishAdapter = communicator->createObjectAdapter(name + ".Publish");

    //
    // We use the name of the service for the name of the database environment.
    //
    string instanceName = properties->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    Identity topicManagerId = { "TopicManager", instanceName };

    if(properties->getPropertyAsIntWithDefault(name+ ".Transient", 0) > 0)
    {
        _instance = make_shared<Instance>(instanceName, name, communicator, publishAdapter, topicAdapter, nullptr);
        try
        {
            auto manager = make_shared<TransientTopicManagerImpl>(_instance);
            _managerProxy = Ice::uncheckedCast<TopicManagerPrx>(topicAdapter->add(manager, topicManagerId));
        }
        catch(const Ice::Exception& ex)
        {
            _instance = 0;

            LoggerOutputBase s;
            s << "exception while starting IceStorm service " << name << ":\n";
            s << ex;
            throw IceBox::FailureException(__FILE__, __LINE__, s.str());
        }
        topicAdapter->activate();
        publishAdapter->activate();
        return;
    }

    if(id == -1) // No replication.
    {
        try
        {
            auto instance = make_shared<PersistentInstance>(instanceName, name, communicator, publishAdapter, topicAdapter);
            _manager = TopicManagerImpl::create(instance);
            _instance = move(instance);
            _managerProxy = uncheckedCast<TopicManagerPrx>(topicAdapter->add(_manager->getServant(), topicManagerId));
        }
        catch(const IceUtil::Exception& ex)
        {
            _instance = 0;

            LoggerOutputBase s;
            s << "exception while starting IceStorm service " << name << ":\n";
            s << ex;

            throw IceBox::FailureException(__FILE__, __LINE__, s.str());
        }
    }
    else
    {
        // Here we want to create a map of id -> election node proxies.
        map<int, shared_ptr<NodePrx>> nodes;

        string topicManagerAdapterId = properties->getProperty(name + ".TopicManager.AdapterId");

        // We support two possible deployments. The first is a manual
        // deployment, the second is IceGrid.
        //
        // Here we check for the manual deployment
        const string prefix = name + ".Nodes.";
        Ice::PropertyDict props = properties->getPropertiesForPrefix(prefix);
        if(!props.empty())
        {
            for(const auto& prop : props)
            {
                try
                {
                    int nodeid = stoi(prop.first.substr(prefix.size()));
                    nodes[nodeid] = uncheckedCast<NodePrx>(communicator->propertyToProxy(prop.first));
                }
                catch(const std::invalid_argument&)
                {
                    Ice::Warning warn(communicator->getLogger());
                    warn << "deployment warning: invalid node id `" << prop.first.substr(prefix.size()) << "'";
                }
            }
        }
        else
        {
            // If adapter id's are defined for the topic manager or
            // node adapters then we consider this an IceGrid based
            // deployment.
            string nodeAdapterId = properties->getProperty(name + ".Node.AdapterId");

            // Validate first that the adapter ids match for the node
            // and the topic manager otherwise some other deployment
            // is being used.
            const string suffix = ".TopicManager";
            if(topicManagerAdapterId.empty() || nodeAdapterId.empty() ||
               topicManagerAdapterId.replace(
                   topicManagerAdapterId.find(suffix), suffix.size(), ".Node") != nodeAdapterId)
            {
                Ice::Error error(communicator->getLogger());
                error << "deployment error: `" << topicManagerAdapterId << "' prefix does not match `"
                      << nodeAdapterId << "'";
                throw IceBox::FailureException(__FILE__, __LINE__, "IceGrid deployment is incorrect");
            }

            // Determine the set of node id and node proxies.
            //
            // This is determined by locating all topic manager
            // replicas, and then working out the node for that
            // replica.
            //
            // We work out the node id by removing the instance
            // name. The node id must follow.
            //
            auto locator = Ice::checkedCast<IceGrid::LocatorPrx>(communicator->getDefaultLocator());
            assert(locator);
            auto query = locator->getLocalQuery();
            auto replicas = query->findAllReplicas(communicator->stringToProxy(instanceName + "/TopicManager"));

            for(Ice::ObjectProxySeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
            for(const auto& replica : replicas)
            {
                string adapterid = replica->ice_getAdapterId();

                // Replace TopicManager with the node endpoint.
                adapterid = adapterid.replace(adapterid.find(suffix), suffix.size(), ".Node");

                // The adapter id must start with the instance name.
                if(adapterid.find(instanceName) != 0)
                {
                    Ice::Error error(communicator->getLogger());
                    error << "deployment error: `" << adapterid << "' does not start with `" << instanceName << "'";
                    throw IceBox::FailureException(__FILE__, __LINE__, "IceGrid deployment is incorrect");
                }

                // The node id follows. We find the first digit (the
                // start of the node id, and then the end of the
                // digits).
                string::size_type start = instanceName.size();
                while(start < adapterid.size() && !IceUtilInternal::isDigit(adapterid[start]))
                {
                    ++start;
                }
                string::size_type end = start;
                while(end < adapterid.size() && IceUtilInternal::isDigit(adapterid[end]))
                {
                    ++end;
                }
                if(start == end)
                {
                    // We must have at least one digit, otherwise there is
                    // some sort of deployment error.
                    Ice::Error error(communicator->getLogger());
                    error << "deployment error: node id does not follow instance name. instance name:"
                          << instanceName << " adapter id: " << adapterid;
                    throw IceBox::FailureException(__FILE__, __LINE__, "IceGrid deployment is incorrect");
                }

                int nodeid = atoi(adapterid.substr(start, end-start).c_str());
                ostringstream os;
                os << "node" << nodeid;
                Ice::Identity ident;
                ident.category = instanceName;
                ident.name = os.str();

                nodes[nodeid] = uncheckedCast<NodePrx>((*p)->ice_adapterId(adapterid)->ice_identity(ident));
            }
        }

        if(nodes.size() < 3)
        {
            Ice::Error error(communicator->getLogger());
            error << "Replication requires at least 3 Nodes";
            throw IceBox::FailureException(__FILE__, __LINE__, "Replication requires at least 3 Nodes");
        }

        try
        {
            // If the node thread pool size is not set then initialize
            // to the number of nodes + 1 and disable thread pool size
            // warnings.
            if(properties->getProperty(name + ".Node.ThreadPool.Size").empty())
            {
                ostringstream os;
                os << nodes.size() + 1;
                properties->setProperty(name + ".Node.ThreadPool.Size", os.str());
                properties->setProperty(name + ".Node.ThreadPool.SizeWarn", "0");
            }
            if(properties->getProperty(name + ".Node.MessageSizeMax").empty())
            {
                properties->setProperty(name + ".Node.MessageSizeMax", "0"); // No limit on data exchanged internally
            }

            auto nodeAdapter = communicator->createObjectAdapter(name + ".Node");
            auto instance = make_shared<PersistentInstance>(instanceName, name, communicator, publishAdapter,
                                                            topicAdapter, nodeAdapter, nodes[id]);
            _instance = instance;

            _instance->observers()->setMajority(static_cast<unsigned int>(nodes.size())/2);

            // Trace replication information.
            auto traceLevels = _instance->traceLevels();
            if(traceLevels->election > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->electionCat);
                out << "I am node " << id << "\n";
                for(const auto& node : nodes)
                {
                    out << "\tnode: " << node.first << " proxy: " << node.second->ice_toString() << "\n";
                }
            }

            if(topicManagerAdapterId.empty())
            {
                // We're not using an IceGrid deployment. Here we need
                // a proxy which is used to create proxies to the
                // replicas later.
                _managerProxy = uncheckedCast<TopicManagerPrx>(topicAdapter->createProxy(topicManagerId));
            }
            else
            {
                // If we're using IceGrid deployment we need to create
                // indirect proxies.
                _managerProxy = uncheckedCast<TopicManagerPrx>(topicAdapter->createIndirectProxy(topicManagerId));
            }

            _manager = TopicManagerImpl::create(instance);
            topicAdapter->add(_manager->getServant(), topicManagerId);

            ostringstream os; // The node object identity.
            os << "node" << id;
            Ice::Identity nodeid;
            nodeid.category = instanceName;
            nodeid.name = os.str();

            auto node = make_shared<NodeI>(_instance, _manager, _managerProxy, id, nodes);
            _instance->setNode(node);
            nodeAdapter->add(node, nodeid);
            nodeAdapter->activate();

            node->start();
        }
        catch(const IceUtil::Exception& ex)
        {
            _instance = 0;

            LoggerOutputBase s;
            s << "exception while starting IceStorm service " << name << ":\n";
            s << ex;

            throw IceBox::FailureException(__FILE__, __LINE__, s.str());
        }
    }

    topicAdapter->add(make_shared<FinderI>(uncheckedCast<TopicManagerPrx>(topicAdapter->createProxy(topicManagerId))),
                      stringToIdentity("IceStorm/Finder"));

    topicAdapter->activate();
    publishAdapter->activate();
}

void
ServiceI::start(const shared_ptr<Communicator>& communicator,
                const shared_ptr<ObjectAdapter>& topicAdapter,
                const shared_ptr<ObjectAdapter>& publishAdapter,
                const string& name,
                const Identity& id,
                const string&)
{
    //
    // For IceGrid we don't validate the properties as all sorts of
    // non-IceStorm properties are included in the prefix.
    //
    //validateProperties(name, communicator->getProperties(), communicator->getLogger());

    // This is for IceGrid only and as such we use a transient
    // implementation of IceStorm.
    string instanceName = communicator->getProperties()->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    _instance = make_shared<Instance>(instanceName, name, communicator, publishAdapter, topicAdapter, nullptr);

    try
    {
        auto manager = make_shared<TransientTopicManagerImpl>(_instance);
        _managerProxy = uncheckedCast<TopicManagerPrx>(topicAdapter->add(manager, id));
    }
    catch(const Ice::Exception& ex)
    {
        _instance = 0;
        LoggerOutputBase s;
        s << "exception while starting IceStorm service " << name << ":\n";
        s << ex;

        throw IceBox::FailureException(__FILE__, __LINE__, s.str());
    }
}

shared_ptr<TopicManagerPrx>
ServiceI::getTopicManager() const
{
    return _managerProxy;
}

void
ServiceI::stop()
{
    // Shutdown the instance. This deactivates all OAs.
    _instance->shutdown();

    //
    // It's necessary to reap all destroyed topics on shutdown.
    //
    if(_manager)
    {
        _manager->shutdown();
    }
    if(_transientManager)
    {
        _transientManager->shutdown();
    }

    //
    // Destroy the instance. This step must occur last.
    //
    _instance->destroy();
}

void
ServiceI::validateProperties(const string& name, const shared_ptr<Properties>& properties, const shared_ptr<Logger>& logger)
{
    static const string suffixes[] =
    {
        "ReplicatedTopicManagerEndpoints",
        "ReplicatedPublishEndpoints",
        "Nodes.*",
        "Transient",
        "NodeId",
        "Flush.Timeout",
        "InstanceName",
        "Election.MasterTimeout",
        "Election.ElectionTimeout",
        "Election.ResponseTimeout",
        "Publish.AdapterId",
        "Publish.Endpoints",
        "Publish.Locator",
        "Publish.PublishedEndpoints",
        "Publish.ReplicaGroupId",
        "Publish.Router",
        "Publish.ThreadPool.Size",
        "Publish.ThreadPool.SizeMax",
        "Publish.ThreadPool.SizeWarn",
        "Publish.ThreadPool.StackSize",
        "Node.AdapterId",
        "Node.Endpoints",
        "Node.Locator",
        "Node.PublishedEndpoints",
        "Node.ReplicaGroupId",
        "Node.Router",
        "Node.ThreadPool.Size",
        "Node.ThreadPool.SizeMax",
        "Node.ThreadPool.SizeWarn",
        "Node.ThreadPool.StackSize",
        "TopicManager.AdapterId",
        "TopicManager.Endpoints",
        "TopicManager.Locator",
        "TopicManager.Proxy",
        "TopicManager.Proxy.EndpointSelection",
        "TopicManager.Proxy.ConnectionCached",
        "TopicManager.Proxy.PreferSecure",
        "TopicManager.Proxy.LocatorCacheTimeout",
        "TopicManager.Proxy.Locator",
        "TopicManager.Proxy.Router",
        "TopicManager.Proxy.CollocationOptimization",
        "TopicManager.PublishedEndpoints",
        "TopicManager.ReplicaGroupId",
        "TopicManager.Router",
        "TopicManager.ThreadPool.Size",
        "TopicManager.ThreadPool.SizeMax",
        "TopicManager.ThreadPool.SizeWarn",
        "TopicManager.ThreadPool.StackSize",
        "Trace.Election",
        "Trace.Replication",
        "Trace.Subscriber",
        "Trace.Topic",
        "Trace.TopicManager",
        "Send.Timeout",
        "Send.QueueSizeMax",
        "Send.QueueSizeMaxPolicy",
        "Discard.Interval",
        "LMDB.Path",
        "LMDB.MapSize"
    };

    vector<string> unknownProps;
    string prefix = name + ".";
    PropertyDict props = properties->getPropertiesForPrefix(prefix);
    for(PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
    {
        bool valid = false;
        for(unsigned int i = 0; i < sizeof(suffixes)/sizeof(*suffixes); ++i)
        {
            string prop = prefix + suffixes[i];
            if(IceUtilInternal::match(p->first, prop))
            {
                valid = true;
                break;
            }
        }
        if(!valid)
        {
            unknownProps.push_back(p->first);
        }
    }

    if(!unknownProps.empty())
    {
        Warning out(logger);
        out << "found unknown properties for IceStorm service '" << name << "':";
        for(vector<string>::const_iterator p = unknownProps.begin(); p != unknownProps.end(); ++p)
        {
            out << "\n    " << *p;
        }
    }
}
