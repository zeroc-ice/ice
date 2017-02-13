// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/PluginFacadeI.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

ICEGRID_API void setRegistryPluginFacade(const RegistryPluginFacadePtr&);

}

namespace
{

class Init
{
public:

    Init()
    {
        IceGrid::setRegistryPluginFacade(new RegistryPluginFacadeI);
    }

    ~Init()
    {
        IceGrid::setRegistryPluginFacade(0);
    }
};

Init init;

bool 
hasAdapter(const CommunicatorDescriptorPtr& descriptor, const string& adapterId)
{
    for(AdapterDescriptorSeq::const_iterator p = descriptor->adapters.begin(); p != descriptor->adapters.end(); ++p)
    {
        if(p->id == adapterId)
        {
            return true;
        }
    }
    return false;
}

}

RegistryPluginFacadeI::RegistryPluginFacadeI()
{
}

ApplicationInfo
RegistryPluginFacadeI::getApplicationInfo(const string& name) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getApplicationInfo(name);
}

ServerInfo
RegistryPluginFacadeI::getServerInfo(const string& serverId) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getServer(serverId)->getInfo(true);
}

AdapterInfoSeq
RegistryPluginFacadeI::getAdapterInfo(const string& adapterId) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterInfo(adapterId);
}

string
RegistryPluginFacadeI::getAdapterServer(const string& adapterId) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterServer(adapterId);
}

string
RegistryPluginFacadeI::getAdapterNode(const string& adapterId) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterNode(adapterId);
}

string
RegistryPluginFacadeI::getAdapterApplication(const string& adapterId) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterApplication(adapterId);
}

ObjectInfo
RegistryPluginFacadeI::getObjectInfo(const Ice::Identity& id) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getObjectInfo(id);
}

NodeInfo
RegistryPluginFacadeI::getNodeInfo(const string& name) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return toNodeInfo(_database->getNode(name)->getInfo());
}

LoadInfo
RegistryPluginFacadeI::getNodeLoad(const string& name) const
{
    Lock sync(*this);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getNode(name)->getSession()->getLoadInfo();
}

std::string
RegistryPluginFacadeI::getPropertyForAdapter(const std::string& adapterId, const std::string& name) const
{
    try
    {
        ServerInfo info = _database->getServer(_database->getAdapterServer(adapterId))->getInfo(true);
        if(hasAdapter(info.descriptor, adapterId))
        {
            return IceGrid::getProperty(info.descriptor->propertySet.properties, name);
        }

        IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(info.descriptor);
        if(!iceBox)
        {
            return "";
        }

        for(ServiceInstanceDescriptorSeq::const_iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
        {
            if(hasAdapter(p->descriptor, adapterId))
            {
                return IceGrid::getProperty(p->descriptor->propertySet.properties, name);
            }
        }
    }
    catch(const ServerNotExistException&)
    {
    }
    catch(const AdapterNotExistException&)
    {
    }
    return "";
}

void
RegistryPluginFacadeI::addReplicaGroupFilter(const string& id, const ReplicaGroupFilterPtr& filter)
{
    Lock sync(*this);
    map<string, vector<ReplicaGroupFilterPtr> >::iterator p = _replicaGroupFilters.find(id);
    if(p == _replicaGroupFilters.end())
    {
        p = _replicaGroupFilters.insert(make_pair(id, vector<ReplicaGroupFilterPtr>())).first;
    }
    p->second.push_back(filter);
}

bool
RegistryPluginFacadeI::removeReplicaGroupFilter(const string& id, const ReplicaGroupFilterPtr& filter)
{
    Lock sync(*this);

    map<string, vector<ReplicaGroupFilterPtr> >::iterator p = _replicaGroupFilters.find(id);
    if(p == _replicaGroupFilters.end())
    {
        return false;
    }

    vector<ReplicaGroupFilterPtr>::iterator q = find(p->second.begin(), p->second.end(), filter);
    if(q == p->second.end())
    {
        return false;
    }
    p->second.erase(q);

    if(p->second.empty())
    {
        _replicaGroupFilters.erase(p);
    }
    return true;
}

void
RegistryPluginFacadeI::addTypeFilter(const string& id, const TypeFilterPtr& filter)
{
    Lock sync(*this);
    map<string, vector<TypeFilterPtr> >::iterator p = _typeFilters.find(id);
    if(p == _typeFilters.end())
    {
        p = _typeFilters.insert(make_pair(id, vector<TypeFilterPtr>())).first;
    }
    p->second.push_back(filter);
}

bool
RegistryPluginFacadeI::removeTypeFilter(const string& id, const TypeFilterPtr& filter)
{
    Lock sync(*this);

    map<string, vector<TypeFilterPtr> >::iterator p = _typeFilters.find(id);
    if(p == _typeFilters.end())
    {
        return false;
    }

    vector<TypeFilterPtr>::iterator q = find(p->second.begin(), p->second.end(), filter);
    if(q == p->second.end())
    {
        return false;
    }
    p->second.erase(q);

    if(p->second.empty())
    {
        _typeFilters.erase(p);
    }
    return true;
}

vector<ReplicaGroupFilterPtr>
RegistryPluginFacadeI::getReplicaGroupFilters(const string& id) const
{
    Lock sync(*this);
    map<string, vector<ReplicaGroupFilterPtr> >::const_iterator p = _replicaGroupFilters.find(id);
    if(p != _replicaGroupFilters.end())
    {
        return p->second;
    }
    return vector<ReplicaGroupFilterPtr>();
}

bool
RegistryPluginFacadeI::hasReplicaGroupFilters() const 
{
    return !_replicaGroupFilters.empty();
}

vector<TypeFilterPtr>
RegistryPluginFacadeI::getTypeFilters(const string& id) const
{
    Lock sync(*this);
    map<string, vector<TypeFilterPtr> >::const_iterator p = _typeFilters.find(id);
    if(p != _typeFilters.end())
    {
        return p->second;
    }
    return vector<TypeFilterPtr>();
}

bool
RegistryPluginFacadeI::hasTypeFilters() const 
{
    return !_typeFilters.empty();
}

void
RegistryPluginFacadeI::setDatabase(const DatabasePtr& database)
{
    Lock sync(*this);
    _database = database;
}
