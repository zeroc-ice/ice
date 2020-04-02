//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceGrid/PluginFacadeI.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/Database.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

ICEGRID_API void setRegistryPluginFacade(const shared_ptr<RegistryPluginFacade>&);

}

namespace
{

class Init
{
public:

    Init()
    {
        IceGrid::setRegistryPluginFacade(make_shared<RegistryPluginFacadeI>());
    }

    ~Init()
    {
        IceGrid::setRegistryPluginFacade(nullptr);
    }
};

Init init;

bool
hasAdapter(const shared_ptr<CommunicatorDescriptor>& descriptor, const string& adapterId)
{
    for(const auto& adapter : descriptor->adapters)
    {
        if(adapter.id == adapterId)
        {
            return true;
        }
    }
    return false;
}

}

ApplicationInfo
RegistryPluginFacadeI::getApplicationInfo(const string& name) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getApplicationInfo(name);
}

ServerInfo
RegistryPluginFacadeI::getServerInfo(const string& serverId) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getServer(serverId)->getInfo(true);
}

AdapterInfoSeq
RegistryPluginFacadeI::getAdapterInfo(const string& adapterId) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterInfo(adapterId);
}

string
RegistryPluginFacadeI::getAdapterServer(const string& adapterId) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterServer(adapterId);
}

string
RegistryPluginFacadeI::getAdapterNode(const string& adapterId) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterNode(adapterId);
}

string
RegistryPluginFacadeI::getAdapterApplication(const string& adapterId) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getAdapterApplication(adapterId);
}

ObjectInfo
RegistryPluginFacadeI::getObjectInfo(const Ice::Identity& id) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return _database->getObjectInfo(id);
}

NodeInfo
RegistryPluginFacadeI::getNodeInfo(const string& name) const
{
    lock_guard lock(_mutex);
    if(!_database)
    {
        throw RegistryUnreachableException("", "registry not initialized yet");
    }
    return toNodeInfo(_database->getNode(name)->getInfo());
}

LoadInfo
RegistryPluginFacadeI::getNodeLoad(const string& name) const
{
    lock_guard lock(_mutex);
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

        auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(info.descriptor);
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
RegistryPluginFacadeI::addReplicaGroupFilter(const string& id, const shared_ptr<ReplicaGroupFilter>& filter) noexcept
{
    lock_guard lock(_mutex);
    map<string, vector<shared_ptr<ReplicaGroupFilter>> >::iterator p = _replicaGroupFilters.find(id);
    if(p == _replicaGroupFilters.end())
    {
        p = _replicaGroupFilters.insert(make_pair(id, vector<shared_ptr<ReplicaGroupFilter>>())).first;
    }
    p->second.push_back(filter);
}

bool
RegistryPluginFacadeI::removeReplicaGroupFilter(const string& id, const shared_ptr<ReplicaGroupFilter>& filter) noexcept
{
    lock_guard lock(_mutex);

    map<string, vector<shared_ptr<ReplicaGroupFilter>> >::iterator p = _replicaGroupFilters.find(id);
    if(p == _replicaGroupFilters.end())
    {
        return false;
    }

    vector<shared_ptr<ReplicaGroupFilter>>::iterator q = find(p->second.begin(), p->second.end(), filter);
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
RegistryPluginFacadeI::addTypeFilter(const string& id, const shared_ptr<TypeFilter>& filter) noexcept
{
    lock_guard lock(_mutex);
    map<string, vector<shared_ptr<TypeFilter>> >::iterator p = _typeFilters.find(id);
    if(p == _typeFilters.end())
    {
        p = _typeFilters.insert(make_pair(id, vector<shared_ptr<TypeFilter>>())).first;
    }
    p->second.push_back(filter);
}

bool
RegistryPluginFacadeI::removeTypeFilter(const string& id, const shared_ptr<TypeFilter>& filter) noexcept
{
    lock_guard lock(_mutex);

    map<string, vector<shared_ptr<TypeFilter>> >::iterator p = _typeFilters.find(id);
    if(p == _typeFilters.end())
    {
        return false;
    }

    vector<shared_ptr<TypeFilter>>::iterator q = find(p->second.begin(), p->second.end(), filter);
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

vector<shared_ptr<ReplicaGroupFilter>>
RegistryPluginFacadeI::getReplicaGroupFilters(const string& id) const
{
    lock_guard lock(_mutex);
    map<string, vector<shared_ptr<ReplicaGroupFilter>> >::const_iterator p = _replicaGroupFilters.find(id);
    if(p != _replicaGroupFilters.end())
    {
        return p->second;
    }
    return vector<shared_ptr<ReplicaGroupFilter>>();
}

bool
RegistryPluginFacadeI::hasReplicaGroupFilters() const
{
    return !_replicaGroupFilters.empty();
}

vector<shared_ptr<TypeFilter>>
RegistryPluginFacadeI::getTypeFilters(const string& id) const
{
    lock_guard lock(_mutex);
    map<string, vector<shared_ptr<TypeFilter>> >::const_iterator p = _typeFilters.find(id);
    if(p != _typeFilters.end())
    {
        return p->second;
    }
    return vector<shared_ptr<TypeFilter>>();
}

bool
RegistryPluginFacadeI::hasTypeFilters() const
{
    return !_typeFilters.empty();
}

void
RegistryPluginFacadeI::setDatabase(const shared_ptr<Database>& database)
{
    lock_guard lock(_mutex);
    _database = database;
}
