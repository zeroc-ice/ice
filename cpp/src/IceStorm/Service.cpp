// **********************************************************************
//
// Copyright (c) 2003-2008 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceStorm/TopicI.h>
#include <IceStorm/TopicManagerI.h>
#include <IceStorm/TransientTopicManagerI.h>
#include <IceStorm/Instance.h>
#include <IceStorm/Service.h>
#include <IceStorm/Observers.h>
#include <IceStorm/TraceLevels.h>
#include <IceUtil/StringUtil.h>

#include <IceStorm/NodeI.h>

#include <IceGrid/Locator.h>
#include <IceGrid/Query.h>

using namespace std;
using namespace Ice;
using namespace Freeze;
using namespace IceStorm;
using namespace IceStormElection;

namespace IceStorm
{

class ServiceI : public ::IceStorm::Service
{
public:

    ServiceI();
    virtual ~ServiceI();

    virtual void start(const string&,
                       const CommunicatorPtr&,
                       const StringSeq&);

    virtual void start(const CommunicatorPtr&, 
                       const ObjectAdapterPtr&, 
                       const ObjectAdapterPtr&,
                       const string&, 
                       const Ice::Identity&,
                       const string&);

    virtual TopicManagerPrx getTopicManager() const;    

    virtual void stop();

private:

    void validateProperties(const string&, const PropertiesPtr&, const LoggerPtr&);

    TopicManagerImplPtr _manager;
    TransientTopicManagerImplPtr _transientManager;
    TopicManagerPrx _managerProxy;
    InstancePtr _instance;
};

}

extern "C"
{

ICE_STORM_API ::IceBox::Service*
createIceStorm(CommunicatorPtr communicator)
{
    return new ServiceI;
}

}

ServicePtr
IceStorm::Service::create(const CommunicatorPtr& communicator,
                          const ObjectAdapterPtr& topicAdapter,
                          const ObjectAdapterPtr& publishAdapter,
                          const string& name,
                          const Ice::Identity& id,
                          const string& dbEnv)
{
    ServiceI* service = new ServiceI;
    ServicePtr svc = service;
    service->start(communicator, topicAdapter, publishAdapter, name, id, dbEnv);
    return svc;
}

IceStorm::ServiceI::ServiceI()
{
}

IceStorm::ServiceI::~ServiceI()
{
}

void
IceStorm::ServiceI::start(
    const string& name,
    const CommunicatorPtr& communicator,
    const StringSeq& args)
{
    PropertiesPtr properties = communicator->getProperties();

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

    Ice::ObjectAdapterPtr topicAdapter = communicator->createObjectAdapter(name + ".TopicManager");
    Ice::ObjectAdapterPtr publishAdapter = communicator->createObjectAdapter(name + ".Publish");

    //
    // We use the name of the service for the name of the database environment.
    //
    string instanceName = properties->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    Identity topicManagerId;
    topicManagerId.category = instanceName;
    topicManagerId.name = "TopicManager";

    if(properties->getPropertyAsIntWithDefault(name+ ".Transient", 0))
    {
        _instance = new Instance(instanceName, name, communicator, publishAdapter, topicAdapter);
        try
        {
            TransientTopicManagerImplPtr manager = new TransientTopicManagerImpl(_instance);
            _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->add(manager, topicManagerId));
        }
        catch(const Ice::Exception&)
        {
            _instance = 0;
            throw;
        }
        topicAdapter->activate();
        publishAdapter->activate();
        return;
    }

    if(id == -1) // No replication.
    {
        _instance = new Instance(instanceName, name, communicator, publishAdapter, topicAdapter);

        try
        {
            _manager = new TopicManagerImpl(_instance);
            _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->add(_manager->getServant(), topicManagerId));
        }
        catch(const Ice::Exception&)
        {
            _instance = 0;
            throw;
        }
    }
    else
    {
        // Here we want to create a map of id -> election node
        // proxies.
        map<int, NodePrx> nodes;

        bool iceGridDeployment = true;

        // We support two possible deployments. The first is a manual
        // deployment, the second is IceGrid.
        //
        // Here we check for the manual deployment 
        const string prefix = name + ".Nodes.";
        Ice::PropertyDict props = properties->getPropertiesForPrefix(prefix);
        if(!props.empty())
        {
            iceGridDeployment = false;
            for(Ice::PropertyDict::const_iterator p = props.begin(); p != props.end(); ++p)
            {
                int nodeid = atoi(p->first.substr(prefix.size()).c_str());
                nodes[nodeid] = NodePrx::uncheckedCast(communicator->propertyToProxy(p->first));
            }
        }
        else
        {
            // If adapter id's are defined for the topic manager or node
            // adapters then we consider this an IceGrid based deployment.
            string adapterid = properties->getProperty(name + ".TopicManager.AdapterId");
            string nodeid = properties->getProperty(name + ".Node.AdapterId");

            // Here I must validate first that the adapter ids match
            // for the node and the topic manager otherwise some other
            // deployment is being used.
            const string suffix = ".TopicManager";
            if(adapterid.empty() || nodeid.empty() ||
               adapterid.replace(adapterid.find(suffix), suffix.size(), ".Node") != nodeid)
            {
                Ice::Error error(communicator->getLogger());
                error << "IceGrid deployment is incorrect";
                throw "IceGrid deployment is incorrect";
            }

            // This is a deployment using IceGrid.
            //
            // In this case we first locate all replicas for the topic
            // manager.  The topic manager adapter ids will be
            // something like:
            // "Inst1-1.IceStorm.IceStorm.TopicManager". This is
            // <instance>-<node-id>.<name>.<endpoint>. From this we
            // can extract node id and create the correct adapter id
            // for the election node.
            IceGrid::LocatorPrx locator = IceGrid::LocatorPrx::checkedCast(communicator->getDefaultLocator());
            assert(locator);
            IceGrid::QueryPrx query = locator->getLocalQuery();
            Ice::ObjectProxySeq replicas = query->findAllReplicas(
                communicator->stringToProxy(instanceName + "/TopicManager"));
            for(Ice::ObjectProxySeq::const_iterator p = replicas.begin(); p != replicas.end(); ++p)
            {
                adapterid = (*p)->ice_getAdapterId();

                // Replace TopicManager with the node endpoint.
                adapterid = adapterid.replace(adapterid.find(suffix), suffix.size(), ".Node");

                // Now look for the node identity. This will be
                // <prefix>-<id>.<name>.<name>.Node... For example, if
                // the service name is IceStorm it will be
                // ".IceStorm.IceStorm.Node"
                string::size_type end = adapterid.find( "." + name + "." + name + ".Node");
                if(end == string::npos)
                {
                    Ice::Error error(communicator->getLogger());
                    error << "IceGrid deployment is incorrect: " << adapterid;
                    throw "IceGrid deployment is incorrect";
                }

                string::size_type start = adapterid.rfind("-", end);
                if(start == string::npos)
                {
                    Ice::Error error(communicator->getLogger());
                    error << "IceGrid deployment is incorrect: " << adapterid;
                    throw "IceGrid deployment is incorrect";
                }
                ++start;

                int nodeid = atoi(adapterid.substr(start, end-start).c_str());
                ostringstream os;
                os << "node" << nodeid;
                Ice::Identity id;
                id.category = instanceName;
                id.name = os.str();

                nodes[nodeid] = NodePrx::uncheckedCast((*p)->ice_adapterId(adapterid)->ice_identity(id));
            }
        }

        if(nodes.size() < 3)
        {
            Ice::Error error(communicator->getLogger());
            error << "Replication requires at least 3 Nodes";
            throw "error";
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
            Ice::ObjectAdapterPtr nodeAdapter = communicator->createObjectAdapter(name + ".Node");

            _instance = new Instance(instanceName, name, communicator, publishAdapter, topicAdapter, iceGridDeployment,
                                     nodeAdapter, nodes[id]);
            _instance->observers()->setMajority(static_cast<unsigned int>(nodes.size())/2);
            
            // Trace replication information.
            TraceLevelsPtr traceLevels = _instance->traceLevels();
            if(traceLevels->election > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->electionCat);
                out << "I am node " << id << "\n";
                for(map<int, NodePrx>::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
                {
                    out << "\tnode: " << p->first << " proxy: " << p->second->ice_toString() << "\n";
                }
            }

            if(!iceGridDeployment)
            {
                // We're not using an IceGrid deployment. Here we need
                // a proxy which is used to create proxies to the
                // replicas later.
                _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->createProxy(topicManagerId));
            }
            else
            {
                // If we're using IceGrid deployment we need to create
                // indirect proxies.
                _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->createIndirectProxy(topicManagerId));
            }
            
            _manager = new TopicManagerImpl(_instance);
            topicAdapter->add(_manager->getServant(), topicManagerId);

            ostringstream os; // The node object identity.
            os << "node" << id;
            Ice::Identity nodeid;
            nodeid.category = instanceName;
            nodeid.name = os.str();

            NodeIPtr node = new NodeI(_instance, _manager, _managerProxy, id, nodes);
            _instance->setNode(node);
            nodeAdapter->add(node, nodeid);
            nodeAdapter->activate();

            node->start();
        }
        catch(const Ice::Exception&)
        {
            _instance = 0;
            throw;
        }
    }
        
    topicAdapter->activate();
    publishAdapter->activate();
}

void
IceStorm::ServiceI::start(const CommunicatorPtr& communicator,
                          const ObjectAdapterPtr& topicAdapter,
                          const ObjectAdapterPtr& publishAdapter,
                          const string& name,
                          const Ice::Identity& id,
                          const string& dbEnv)
{
    //
    // For IceGrid we don't validate the properties as all sorts of
    // non-IceStorm properties are included in the prefix.
    //
    //validateProperties(name, communicator->getProperties(), communicator->getLogger());

    // This is for IceGrid only and as such we use a transient
    // implementation of IceStorm.
    string instanceName = communicator->getProperties()->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    _instance = new Instance(instanceName, name, communicator, publishAdapter, topicAdapter);

    try
    {
        TransientTopicManagerImplPtr manager = new TransientTopicManagerImpl(_instance);
        _managerProxy = TopicManagerPrx::uncheckedCast(topicAdapter->add(manager, id));
    }
    catch(const Ice::Exception&)
    {
        _instance = 0;
        throw;
    }
}

TopicManagerPrx
IceStorm::ServiceI::getTopicManager() const
{
    return _managerProxy;
}

void
IceStorm::ServiceI::stop()
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
IceStorm::ServiceI::validateProperties(const string& name, const PropertiesPtr& properties, const LoggerPtr& logger)
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
        "Publish.RegisterProcess",
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
        "Node.RegisterProcess",
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
        "TopicManager.RegisterProcess",
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
        "Discard.Interval",
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

