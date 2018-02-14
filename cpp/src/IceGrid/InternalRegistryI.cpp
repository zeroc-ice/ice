// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Ice/Ice.h>

#include <IceGrid/RegistryI.h>
#include <IceGrid/InternalRegistryI.h>
#include <IceGrid/Database.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Topics.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/FileCache.h>
#include <IceSSL/IceSSL.h>
#include <IceSSL/RFC2253.h>

using namespace std;
using namespace IceGrid;

InternalRegistryI::InternalRegistryI(const RegistryIPtr& registry,
                                     const DatabasePtr& database, 
                                     const ReapThreadPtr& reaper,
                                     const WellKnownObjectsManagerPtr& wellKnownObjects,
                                     ReplicaSessionManager& session) : 
    _registry(registry),
    _database(database),
    _reaper(reaper),
    _wellKnownObjects(wellKnownObjects),
    _fileCache(new FileCache(database->getCommunicator())),
    _session(session)
{
    Ice::PropertiesPtr properties = database->getCommunicator()->getProperties();
    _nodeSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 30);
    _replicaSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.ReplicaSessionTimeout", 30);
    _requireNodeCertCN = properties->getPropertyAsIntWithDefault("IceGrid.Registry.RequireNodeCertCN", 0);
    _requireReplicaCertCN = properties->getPropertyAsIntWithDefault("IceGrid.Registry.RequireReplicaCertCN", 0);
}

InternalRegistryI::~InternalRegistryI()
{
}

NodeSessionPrx
InternalRegistryI::registerNode(const InternalNodeInfoPtr& info, 
                                const NodePrx& node, 
                                const LoadInfo& load, 
                                const Ice::Current& current)
{
    const TraceLevelsPtr traceLevels = _database->getTraceLevels();
    const Ice::LoggerPtr logger = traceLevels->logger;
    if(!info || !node)
    {
        return 0;
    }

    if(_requireNodeCertCN)
    {
        try
        {
            IceSSL::ConnectionInfoPtr sslConnInfo = IceSSL::ConnectionInfoPtr::dynamicCast(current.con->getInfo());
            if(sslConnInfo)
            {
                if (sslConnInfo->certs.empty() ||
                    !IceSSL::Certificate::decode(sslConnInfo->certs[0])->getSubjectDN().match("CN=" + info->name))
                {
                    if(traceLevels->node > 0)
                    {
                        Ice::Trace out(logger, traceLevels->nodeCat);
                        out << "certificate CN doesn't match node name `" << info->name << "'";
                    }
                    throw PermissionDeniedException("certificate CN doesn't match node name `" + info->name + "'");
                }
            }
            else
            {
                if(traceLevels->node > 0)
                {
                    Ice::Trace out(logger, traceLevels->nodeCat);
                    out << "node certificate for `" << info->name << "' is required to connect to this registry";
                }
                throw PermissionDeniedException("node certificate is required to connect to this registry");
            }
        }
        catch(const PermissionDeniedException& ex)
        {
            throw ex;
        }
        catch(const IceUtil::Exception&)
        {
            if(traceLevels->node > 0)
            {
                Ice::Trace out(logger, traceLevels->nodeCat);
                out << "unexpected exception while verifying certificate for node `" << info->name << "'";
            }
            throw PermissionDeniedException("unable to verify certificate for node `" + info->name + "'");
        }
    }
 
    try
    {
        NodeSessionIPtr session = new NodeSessionI(_database, node, info, _nodeSessionTimeout, load);
        _reaper->add(new SessionReapable<NodeSessionI>(logger, session), _nodeSessionTimeout);
        return session->getProxy();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }
}

ReplicaSessionPrx
InternalRegistryI::registerReplica(const InternalReplicaInfoPtr& info,
                                   const InternalRegistryPrx& prx,
                                   const Ice::Current& current)
{
    const TraceLevelsPtr traceLevels = _database->getTraceLevels();
    const Ice::LoggerPtr logger = traceLevels->logger;
    if(!info || !prx)
    {
        return 0;
    }

    if(_requireReplicaCertCN)
    {
        try
        {
            IceSSL::ConnectionInfoPtr sslConnInfo = IceSSL::ConnectionInfoPtr::dynamicCast(current.con->getInfo());
            if(sslConnInfo)
            {
                if (sslConnInfo->certs.empty() ||
                    !IceSSL::Certificate::decode(sslConnInfo->certs[0])->getSubjectDN().match("CN=" + info->name))
                {
                    if(traceLevels->replica > 0)
                    {
                        Ice::Trace out(logger, traceLevels->replicaCat);
                        out << "certificate CN doesn't match replica name `" << info->name << "'";
                    }
                    throw PermissionDeniedException("certificate CN doesn't match replica name `" + info->name + "'");
                }
            }
            else
            {
                if(traceLevels->replica > 0)
                {
                    Ice::Trace out(logger, traceLevels->replicaCat);
                    out << "replica certificate for `" << info->name << "' is required to connect to this registry";
                }
                throw PermissionDeniedException("replica certificate is required to connect to this registry");
            }
        }
        catch(const PermissionDeniedException& ex)
        {
            throw ex;
        }
        catch(const IceUtil::Exception&)
        {
            if(traceLevels->replica > 0)
            {
                Ice::Trace out(logger, traceLevels->replicaCat);
                out << "unexpected exception while verifying certificate for replica `" << info->name << "'";
            }
            throw PermissionDeniedException("unable to verify certificate for replica `" + info->name + "'");
        }
    }
    
    try
    {
        ReplicaSessionIPtr s = new ReplicaSessionI(_database, _wellKnownObjects, info, prx, _replicaSessionTimeout);
        _reaper->add(new SessionReapable<ReplicaSessionI>(logger, s), _replicaSessionTimeout);
        return s->getProxy();
    }
    catch(const Ice::ObjectAdapterDeactivatedException&)
    {
        throw Ice::ObjectNotExistException(__FILE__, __LINE__, current.id, current.facet, current.operation);
    }
}

void
InternalRegistryI::registerWithReplica(const InternalRegistryPrx& replica, const Ice::Current&)
{
    _session.create(replica);
}

NodePrxSeq
InternalRegistryI::getNodes(const Ice::Current&) const
{
    NodePrxSeq nodes;
    Ice::ObjectProxySeq proxies = _database->getInternalObjectsByType(Node::ice_staticId());
    for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        nodes.push_back(NodePrx::uncheckedCast(*p));
    }
    return nodes;
}

InternalRegistryPrxSeq
InternalRegistryI::getReplicas(const Ice::Current&) const
{
    InternalRegistryPrxSeq replicas;
    Ice::ObjectProxySeq proxies = _database->getObjectsByType(InternalRegistry::ice_staticId());
    for(Ice::ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        replicas.push_back(InternalRegistryPrx::uncheckedCast(*p));
    }
    return replicas;
}

ApplicationInfoSeq 
InternalRegistryI::getApplications(Ice::Long& serial, const Ice::Current&) const
{
    return _database->getApplications(serial);
}

AdapterInfoSeq 
InternalRegistryI::getAdapters(Ice::Long& serial, const Ice::Current&) const
{
    return _database->getAdapters(serial);
}

ObjectInfoSeq 
InternalRegistryI::getObjects(Ice::Long& serial, const Ice::Current&) const
{
    return _database->getObjects(serial);
}

void
InternalRegistryI::shutdown(const Ice::Current& /*current*/) const
{
    _registry->shutdown();
}

Ice::Long
InternalRegistryI::getOffsetFromEnd(const string& filename, int count, const Ice::Current&) const
{
    return _fileCache->getOffsetFromEnd(getFilePath(filename), count);
}

bool
InternalRegistryI::read(const string& filename, Ice::Long pos, int size, Ice::Long& newPos, Ice::StringSeq& lines, 
                        const Ice::Current&) const
{
    return _fileCache->read(getFilePath(filename), pos, size, newPos, lines);
}

string
InternalRegistryI::getFilePath(const string& filename) const
{
    string file;
    if(filename == "stderr")
    {
        file = _database->getCommunicator()->getProperties()->getProperty("Ice.StdErr");
        if(file.empty())
        {
            throw FileNotAvailableException("Ice.StdErr configuration property is not set");
        }
    }
    else if(filename == "stdout")
    {
        file = _database->getCommunicator()->getProperties()->getProperty("Ice.StdOut");
        if(file.empty())
        {
            throw FileNotAvailableException("Ice.StdOut configuration property is not set");
        }
    }
    else
    {
        throw FileNotAvailableException("unknown file");
    }
    return file;
}
