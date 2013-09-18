// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ArgVector.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Communicator.h>
#include <Ice/Locator.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <IceDB/SqlTypes.h>
#include <IceGrid/SqlDB/SqlDB.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>

using namespace IceGrid;
using namespace std;

extern "C"
{

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createSqlDB(const Ice::CommunicatorPtr& communicator, const string& name, const Ice::StringSeq& args)
{
    IceUtilInternal::ArgVector av(args);
    return new IceGrid::SqlDBPlugin(communicator, av.argc, av.argv);
}

}

namespace
{

//
// SQL wrappers for SQL tables
//

class SqlApplicationsWrapper : public SqlDB::Wrapper<SqlStringApplicationInfoDict, std::string, ApplicationInfo>, 
                               public ApplicationsWrapper
{
public:

    SqlApplicationsWrapper(const SqlDB::DatabaseConnectionPtr& con, const SqlStringApplicationInfoDictPtr& table) :
        SqlDB::Wrapper<SqlStringApplicationInfoDict, std::string, ApplicationInfo>(con, table)
    {
    }

    virtual Ice::Long
    updateSerial(Ice::Long serial)
    {
        return -1; // Serials not supported
    }

    virtual Ice::Long
    getSerial() const
    {
        return -1; // Serials not supported
    }
};

class SqlAdaptersWrapper : public SqlDB::Wrapper<SqlStringAdapterInfoDict, std::string, AdapterInfo>, 
                           public AdaptersWrapper
{
public:

    SqlAdaptersWrapper(const SqlDB::DatabaseConnectionPtr& connection, const SqlStringAdapterInfoDictPtr& table) :
        SqlDB::Wrapper<SqlStringAdapterInfoDict, std::string, AdapterInfo>(connection, table)
    {
    }
    
    virtual std::vector<AdapterInfo>
    findByReplicaGroupId(const std::string& name)
    {
        return _table->findByReplicaGroupId(_connection, name);
    }

    virtual Ice::Long
    updateSerial(Ice::Long serial)
    {
        return -1; // Serials not supported
    }

    virtual Ice::Long
    getSerial() const
    {
        return -1; // Serials not supported
    }
};

class SqlObjectsWrapper : public SqlDB::Wrapper<SqlIdentityObjectInfoDict, Ice::Identity, ObjectInfo>,
                          public ObjectsWrapper
{
public:

    SqlObjectsWrapper(const SqlDB::DatabaseConnectionPtr& connection, const SqlIdentityObjectInfoDictPtr& table) :
        SqlDB::Wrapper<SqlIdentityObjectInfoDict, Ice::Identity, ObjectInfo>(connection, table)
    {
    }

    virtual std::vector<ObjectInfo>
    findByType(const std::string& type)
    {
        return _table->findByType(_connection, type);
    }

    virtual Ice::Long
    updateSerial(Ice::Long serial)
    {
        return -1; // Serials not supported
    }

    virtual Ice::Long
    getSerial() const
    {
        return -1; // Serials not supported
    }
};

}

SqlConnectionPool::SqlConnectionPool(const Ice::CommunicatorPtr& communicator, 
                                     const string& databaseType,
                                     const string& databaseName,
                                     const string& hostname,
                                     int port,
                                     const string& username,
                                     const string& password,
                                     const string& tablePrefix,
                                     const string& encoding) :
    SqlDB::ConnectionPool(communicator, databaseType, databaseName, hostname, port, username, password, true,
                          Ice::stringToEncodingVersion(encoding))
{
    IceDB::DatabaseConnectionPtr connection = getConnection();
    IceDB::TransactionHolder txn(connection);

    SqlDB::DatabaseConnectionPtr c = SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get());

    const_cast<SqlStringApplicationInfoDictPtr&>(_applications) = 
        new SqlStringApplicationInfoDict(c, tablePrefix + "Applications", communicator);
    const_cast<SqlStringAdapterInfoDictPtr&>(_adapters) = 
        new SqlStringAdapterInfoDict(c, tablePrefix + "Adapters", communicator);
    const_cast<SqlIdentityObjectInfoDictPtr&>(_objects) =
        new SqlIdentityObjectInfoDict(c, tablePrefix + "Objects", communicator);
    const_cast<SqlIdentityObjectInfoDictPtr&>(_internalObjects) =
        new SqlIdentityObjectInfoDict(c, tablePrefix + "InternalObjects", communicator);
    
    txn.commit();
}

SqlConnectionPool::~SqlConnectionPool()
{
}

ApplicationsWrapperPtr 
SqlConnectionPool::getApplications(const IceDB::DatabaseConnectionPtr& connection)
{
    return new SqlApplicationsWrapper(SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get()), _applications);
}

AdaptersWrapperPtr 
SqlConnectionPool::getAdapters(const IceDB::DatabaseConnectionPtr& connection)
{
    return new SqlAdaptersWrapper(SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get()), _adapters);
}

ObjectsWrapperPtr 
SqlConnectionPool::getObjects(const IceDB::DatabaseConnectionPtr& connection)
{
    return new SqlObjectsWrapper(SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get()), _objects);
}

ObjectsWrapperPtr 
SqlConnectionPool::getInternalObjects(const IceDB::DatabaseConnectionPtr& connection)
{
    return new SqlObjectsWrapper(SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get()), _internalObjects);
}

SqlDBPlugin::SqlDBPlugin(const Ice::CommunicatorPtr& communicator, int argc, char** argv) :
    _communicator(communicator), _qtApp(0)
{
    if(QCoreApplication::instance() == 0)
    {
        _qtApp = new QCoreApplication(argc, argv);
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    }
    
    Ice::PluginPtr(new Ice::ThreadHookPlugin(_communicator, new SqlDB::ThreadHook()));
}

SqlDBPlugin::~SqlDBPlugin()
{
    if(_qtApp != 0)
    {
        delete _qtApp;
        _qtApp = 0;
    }
}

void
SqlDBPlugin::initialize()
{
}

void
SqlDBPlugin::destroy()
{
    //
    // Break cyclic reference count (thread hook holds a reference on the cache and the cache holds
    // a reference on the communicator through the SQL dictionaries).
    //
    SqlDB::ThreadHookPtr threadHook = 
        SqlDB::ThreadHookPtr::dynamicCast(IceInternal::getInstance(_communicator)->initializationData().threadHook);
    if(threadHook)
    {
        threadHook->setConnectionPool(0);
    }
    _connectionPool = 0;
}

bool
SqlDBPlugin::initDB()
{
    Ice::PropertiesPtr properties = _communicator->getProperties();
    string databaseName;
    string tablePrefix;
    if(properties->getProperty("IceGrid.SQL.DatabaseType") == "QSQLITE")
    {
        string dbPath = properties->getProperty("IceGrid.Registry.Data");
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
        databaseName = dbPath + "/" + properties->getPropertyWithDefault("IceGrid.SQL.DatabaseName", "registry.db");
    }
    else
    {
        databaseName = properties->getProperty("IceGrid.SQL.DatabaseName");

        string replicaName = properties->getPropertyWithDefault("IceGrid.Registry.ReplicaName", "Master");
        string instanceName;
        if(_communicator->getDefaultLocator())
        {
            instanceName = _communicator->getDefaultLocator()->ice_getIdentity().category;
        }
        else
        {
            instanceName = properties->getPropertyWithDefault("IceGrid.InstanceName", "IceGrid");
        }

        tablePrefix = instanceName + "_" + replicaName + "_";

        replace(tablePrefix.begin(), tablePrefix.end(), '.', '_');
        replace(tablePrefix.begin(), tablePrefix.end(), '-', '_');
        replace(tablePrefix.begin(), tablePrefix.end(), ' ', '_');
        replace(tablePrefix.begin(), tablePrefix.end(), ';', '_');
    }

    string encodingVersionString = 
        properties->getPropertyWithDefault("IceGrid.SQL.EncodingVersion",
                                           encodingVersionToString(Ice::currentEncoding));

    _connectionPool = new SqlConnectionPool(_communicator, 
                                            properties->getProperty("IceGrid.SQL.DatabaseType"),
                                            databaseName,
                                            properties->getProperty("IceGrid.SQL.HostName"),
                                            properties->getPropertyAsInt("IceGrid.SQL.Port"),
                                            properties->getProperty("IceGrid.SQL.UserName"),
                                            properties->getProperty("IceGrid.SQL.Password"),
                                            tablePrefix,
                                            encodingVersionString);

    SqlDB::ThreadHookPtr threadHook = 
        SqlDB::ThreadHookPtr::dynamicCast(IceInternal::getInstance(_communicator)->initializationData().threadHook);
    assert(threadHook);
    threadHook->setConnectionPool(_connectionPool);
    return true;
}

ConnectionPoolPtr
SqlDBPlugin::getConnectionPool()
{
    return _connectionPool;
}
