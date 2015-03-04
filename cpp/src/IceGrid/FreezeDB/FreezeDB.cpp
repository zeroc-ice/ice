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
#include <IceGrid/FreezeDB/SerialsDict.h>

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

Ice::Long
updateSerialDB(const Freeze::ConnectionPtr& connection, const std::string& dbName, Ice::Long serial)
{
    if(serial == -1) // Master doesn't support serials.
    {
        return -1;
    }

    SerialsDict dict(connection, "serials");

    //
    // If a serial number is provided, juste update the serial number from the database, 
    // otherwise if the serial is 0, we increment the serial from the database.
    // 
    SerialsDict::iterator p = dict.find(dbName);
    if(p == dict.end())
    {
        dict.insert(SerialsDict::value_type(dbName, serial == 0 ? 1 : serial));
        return 1;
    }
    else
    {
        p.set(serial == 0 ? p->second + 1 : serial);
        return p->second;
    }
}

Ice::Long
getSerialDB(const Freeze::ConnectionPtr& connection, const std::string& dbName)
{
    SerialsDict dict(connection, "serials");

    //
    // If a serial number is provided, juste update the serial number from the database, 
    // otherwise if the serial is 0, we increment the serial from the database.
    // 
    SerialsDict::iterator p = dict.find(dbName);
    if(p == dict.end())
    {
        dict.insert(SerialsDict::value_type(dbName, 1));
        return 1;
    }
    return p->second;
}

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

    virtual Ice::Long
    updateSerial(Ice::Long serial)
    {
        return updateSerialDB(_dict.getConnection(), _dbName, serial);
    }
    
    virtual Ice::Long
    getSerial() const
    {
        return getSerialDB(_dict.getConnection(), _dbName);
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

    virtual Ice::Long
    updateSerial(Ice::Long serial)
    {
        return updateSerialDB(_dict.getConnection(), _dbName, serial);
    }

    virtual Ice::Long
    getSerial() const
    {
        return getSerialDB(_dict.getConnection(), _dbName);
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

    virtual Ice::Long
    updateSerial(Ice::Long serial)
    {
        return updateSerialDB(_dict.getConnection(), _dbName, serial);
    }

    virtual Ice::Long
    getSerial() const
    {
        return getSerialDB(_dict.getConnection(), _dbName);
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

map<string, Ice::Long>
FreezeConnectionPool::getSerials() const
{
    map<string, Ice::Long> serials;
    try
    {
        IceDB::DatabaseConnectionPtr connection = const_cast<FreezeConnectionPool*>(this)->newConnection();
        FreezeDB::DatabaseConnection* c = dynamic_cast<FreezeDB::DatabaseConnection*>(connection.get());
        SerialsDict dict(c->freezeConnection(), "serials");
        for(SerialsDict::const_iterator p = dict.begin(); p != dict.end(); ++p)
        {
            serials.insert(pair<string, Ice::Long>(p->first, p->second));
        }
    }
    catch(const Freeze::DatabaseException& ex)
    {
        FreezeDB::throwDatabaseException(__FILE__, __LINE__, ex);
    }    
    return serials;
}

FreezeDBPlugin::FreezeDBPlugin(const Ice::CommunicatorPtr& communicator) : _communicator(communicator)
{
}

void
FreezeDBPlugin::initialize()
{
}

void
FreezeDBPlugin::destroy()
{
    _connectionPool = 0;
}

bool
FreezeDBPlugin::initDB()
{
    string dbPath = _communicator->getProperties()->getProperty("IceGrid.Registry.Data");
    if(dbPath.empty())
    {
        Ice::Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Data' is not set";
        return false;
    }
    else
    {
        if(!IceUtilInternal::directoryExists(dbPath))
        {            
            Ice::SyscallException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSystemErrno();

            Ice::Error out(_communicator->getLogger());
            out << "property `IceGrid.Registry.Data' is set to an invalid path:\n" << ex;
            return false;
        }
    }

    _communicator->getProperties()->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);

    _connectionPool = new FreezeConnectionPool(_communicator);

    return true;
}

ConnectionPoolPtr
FreezeDBPlugin::getConnectionPool()
{
    return _connectionPool;
}
