// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/ArgVector.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/Instance.h>

#include <IceStorm/SqlDB/SqlDB.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QTextCodec>

using namespace IceStorm;
using namespace std;

extern "C"
{

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createSqlDB(const Ice::CommunicatorPtr& communicator, const string& name, const Ice::StringSeq& args)
{
    IceUtilInternal::ArgVector av(args);
    return new IceStorm::SqlDBPlugin(communicator, av.argc, av.argv);
}

}

namespace
{

//
// SQL wrappers for SQL tables
//

class SqlLLUWrapper : public LLUWrapper
{
public:

    SqlLLUWrapper(const SqlDB::DatabaseConnectionPtr& connection, const SqlLLUPtr& llu) :
        _connection(connection), _table(llu)
    {
    }

    virtual void 
    put(const IceStormElection::LogUpdate& update)
    {
        _table->put(_connection, update);
    }

    virtual IceStormElection::LogUpdate
    get()
    {
        return _table->get(_connection);
    }

private:

    const SqlDB::DatabaseConnectionPtr _connection;
    const SqlLLUPtr _table;
};

class SqlSubscribersWrapper : public SqlDB::Wrapper<SqlSubscriberMap, SubscriberRecordKey, SubscriberRecord>,
                              public SubscribersWrapper
{
public:

    SqlSubscribersWrapper(const SqlDB::DatabaseConnectionPtr& connection, const SqlSubscriberMapPtr& subscribers) :
        SqlDB::Wrapper<SqlSubscriberMap, SubscriberRecordKey, SubscriberRecord>(connection, subscribers)
    {
    }

    void
    eraseTopic(const Ice::Identity& topic)
    {
        _table->eraseTopic(_connection, topic);
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
    SqlDB::ConnectionPool(communicator, databaseType, databaseName, hostname, port, username, password, false,
                          Ice::stringToEncodingVersion(encoding))
{
    IceDB::DatabaseConnectionPtr connection = getConnection();
    IceDB::TransactionHolder txn(connection);

    SqlDB::DatabaseConnectionPtr c = SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get());

    const_cast<SqlLLUPtr&>(_llu) = new SqlLLU(c, tablePrefix + "LLU");
    const_cast<SqlSubscriberMapPtr&>(_subscribers) = new SqlSubscriberMap(c, tablePrefix + "Subscribers", communicator);
    
    txn.commit();
}

SqlConnectionPool::~SqlConnectionPool()
{
}

LLUWrapperPtr 
SqlConnectionPool::getLLU(const IceDB::DatabaseConnectionPtr& connection)
{
    return new SqlLLUWrapper(SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get()), _llu);
}

SubscribersWrapperPtr 
SqlConnectionPool::getSubscribers(const IceDB::DatabaseConnectionPtr& connection)
{
    return new SqlSubscribersWrapper(SqlDB::DatabaseConnectionPtr::dynamicCast(connection.get()), _subscribers);
}

SqlDBPlugin::SqlDBPlugin(const Ice::CommunicatorPtr& communicator, int argc, char** argv) : _communicator(communicator)
{
    //
    // In order to load SQL drivers it is necessary for an instance of
    // QCoreApplication to be instantiated. However only one can be instantiated
    // per process. Therefore we do not destroy _qtApp as it may be required
    // by other services that are also using QT.
    //
    if(QCoreApplication::instance() == 0)
    {
        new QCoreApplication(argc, argv);
        QTextCodec::setCodecForCStrings(QTextCodec::codecForName("UTF-8"));
    }
    
    Ice::PluginPtr(new Ice::ThreadHookPlugin(_communicator, new SqlDB::ThreadHook()));
}

SqlDBPlugin::~SqlDBPlugin()
{
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
}

ConnectionPoolPtr
SqlDBPlugin::getConnectionPool(const string& name)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    string tablePrefix = properties->getPropertyWithDefault(name + ".InstanceName", "IceStorm");
    int id = properties->getPropertyAsIntWithDefault(name + ".NodeId", -1);
    if(id != -1)
    {
        ostringstream os;
        os << tablePrefix << "_" << id;
        tablePrefix = os.str();
        replace(tablePrefix.begin(), tablePrefix.end(), '.', '_');
        replace(tablePrefix.begin(), tablePrefix.end(), '-', '_');
        replace(tablePrefix.begin(), tablePrefix.end(), ' ', '_');
        replace(tablePrefix.begin(), tablePrefix.end(), ';', '_');
    }
    tablePrefix += "_";

    
    string encodingVersionString = 
        properties->getPropertyWithDefault(name + ".SQL.EncodingVersion",
                                           encodingVersionToString(Ice::currentEncoding));


    SqlConnectionPoolPtr connectionPool = new SqlConnectionPool(_communicator, 
                                                                properties->getProperty(name + ".SQL.DatabaseType"),
                                                                properties->getProperty(name + ".SQL.DatabaseName"),
                                                                properties->getProperty(name + ".SQL.HostName"),
                                                                properties->getPropertyAsInt(name + ".SQL.Port"),
                                                                properties->getProperty(name + ".SQL.UserName"),
                                                                properties->getProperty(name + ".SQL.Password"),
                                                                tablePrefix,
                                                                encodingVersionString);
    
    SqlDB::ThreadHookPtr threadHook = 
        SqlDB::ThreadHookPtr::dynamicCast(IceInternal::getInstance(_communicator)->initializationData().threadHook);
    assert(threadHook);
    threadHook->setConnectionPool(connectionPool);
    return connectionPool;
}
