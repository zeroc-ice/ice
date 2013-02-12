// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/FreezeDB/FreezeDB.h>

#include <IceGrid/FreezeDB/StringApplicationInfoDict.h>
#include <IceGrid/FreezeDB/StringAdapterInfoDict.h>
#include <IceGrid/FreezeDB/IdentityObjectInfoDict.h>

#include <IceUtil/FileUtil.h>

using namespace IceGrid;
using namespace std;

extern "C"
{

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createFreezeDB(const Ice::CommunicatorPtr& communicator, const string& /*name*/, const Ice::StringSeq& /*args*/)
{
    return new IceGrid::FreezeDBPlugin(communicator);
}

}

namespace
{

//
// Freeze wrappers for Freeze dictionaries
//

class FreezeApplicationsWrapper : public FreezeDB::Wrapper<StringApplicationInfoDict, std::string, ApplicationInfo>,
                                  public ApplicationsWrapper
{
public:

    FreezeApplicationsWrapper(const Freeze::ConnectionPtr& connection, const std::string& dbName) :
        FreezeDB::Wrapper<StringApplicationInfoDict, std::string, ApplicationInfo>(connection, dbName)
    {
    }
};

class FreezeAdaptersWrapper : public FreezeDB::Wrapper<StringAdapterInfoDict, std::string, AdapterInfo>,
                              public AdaptersWrapper
{
public:

    FreezeAdaptersWrapper(const Freeze::ConnectionPtr& connection, const std::string& dbName) :
        FreezeDB::Wrapper<StringAdapterInfoDict, std::string, AdapterInfo>(connection, dbName)
    {
    }

    std::vector<AdapterInfo>
    findByReplicaGroupId(const std::string& name)
    {
        std::vector<AdapterInfo> result;
        for(StringAdapterInfoDict::const_iterator p = _dict.findByReplicaGroupId(name, true); p != _dict.end(); ++p)
        {
            result.push_back(p->second);
        }
        return result;
    }
};

class FreezeObjectsWrapper : public FreezeDB::Wrapper<IdentityObjectInfoDict, Ice::Identity, ObjectInfo>,
                             public ObjectsWrapper
{
public:

    FreezeObjectsWrapper(const Freeze::ConnectionPtr& connection, const std::string& name) :
        FreezeDB::Wrapper<IdentityObjectInfoDict, Ice::Identity, ObjectInfo>(connection, name)
    {
    }

    std::vector<ObjectInfo>
    findByType(const std::string& type)
    {
        std::vector<ObjectInfo> result;
        for(IdentityObjectInfoDict::const_iterator p = _dict.findByType(type); p != _dict.end(); ++p)
        {
            result.push_back(p->second);
        }
        return result;
    }
};

}

FreezeConnectionPool::FreezeConnectionPool(const Ice::CommunicatorPtr& communicator) :
    FreezeDB::ConnectionPool(communicator, "Registry")
{
}

ApplicationsWrapperPtr
FreezeConnectionPool::getApplications(const IceDB::DatabaseConnectionPtr& connection)
{
    FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
    // COMPILERFIX: GCC 4.4 w/ -O2 emits strict aliasing warnings
    // without the follow temporary.
    ApplicationsWrapper *w = new FreezeApplicationsWrapper(c->freezeConnection(), "applications");
    return w;
}

AdaptersWrapperPtr
FreezeConnectionPool::getAdapters(const IceDB::DatabaseConnectionPtr& connection)
{
    FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
    return new FreezeAdaptersWrapper(c->freezeConnection(), "adapters");
}

ObjectsWrapperPtr
FreezeConnectionPool::getObjects(const IceDB::DatabaseConnectionPtr& connection)
{
    FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
    return new FreezeObjectsWrapper(c->freezeConnection(), "objects");
}

ObjectsWrapperPtr
FreezeConnectionPool::getInternalObjects(const IceDB::DatabaseConnectionPtr& connection)
{
    FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
    return new FreezeObjectsWrapper(c->freezeConnection(), "internal-objects");
}

FreezeDBPlugin::FreezeDBPlugin(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
    string dbPath = _communicator->getProperties()->getProperty("IceGrid.Registry.Data");
    if(dbPath.empty())
    {
        throw Ice::PluginInitializationException(__FILE__, __LINE__, "property `IceGrid.Registry.Data' is not set");
    }
    else
    {
        if(!IceUtilInternal::directoryExists(dbPath))
        {
            ostringstream os;
            Ice::SyscallException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSystemErrno();
            os << "property `IceGrid.Registry.Data' is set to an invalid path:\n" << ex;
            throw Ice::PluginInitializationException(__FILE__, __LINE__, os.str());
        }
    }

    _communicator->getProperties()->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);
}

void
FreezeDBPlugin::initialize()
{
    _connectionPool = new FreezeConnectionPool(_communicator);
}

void
FreezeDBPlugin::destroy()
{
    _connectionPool = 0;
}

ConnectionPoolPtr
FreezeDBPlugin::getConnectionPool()
{
    return _connectionPool;
}
