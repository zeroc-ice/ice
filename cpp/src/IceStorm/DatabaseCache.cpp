// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/DatabaseCache.h>
#include <IceStorm/Instance.h>
#ifdef QTSQL
#  include <Ice/Communicator.h>
#  include <Ice/Properties.h>
#endif

using namespace IceStorm;
using namespace std;

#ifdef QTSQL

DatabaseCache::DatabaseCache(const InstancePtr& instance) :
    IceSQL::DatabaseCache(instance->communicator(),
        instance->communicator()->getProperties()->getProperty(instance->serviceName() + ".SQL.DatabaseType"),
        instance->communicator()->getProperties()->getProperty(instance->serviceName() + ".SQL.DatabaseName"),
        instance->communicator()->getProperties()->getProperty(instance->serviceName() + ".SQL.HostName"),
        instance->communicator()->getProperties()->getProperty(instance->serviceName() + ".SQL.UserName"),
        instance->communicator()->getProperties()->getProperty(instance->serviceName() + ".SQL.Password"),
        false)
{
    Ice::PropertiesPtr properties = instance->properties();
    string serviceName = instance->serviceName();

    string tablePrefix = properties->getPropertyWithDefault(serviceName + ".InstanceName", "IceStorm");
    string id = properties->getProperty(serviceName + ".NodeId");
    if(!id.empty())
    {
        tablePrefix += "_";
        tablePrefix += id;
    }
    replace(tablePrefix.begin(), tablePrefix.end(), '.', '_');
    replace(tablePrefix.begin(), tablePrefix.end(), '-', '_');
    replace(tablePrefix.begin(), tablePrefix.end(), ' ', '_');
    replace(tablePrefix.begin(), tablePrefix.end(), ';', '_');

    IceSQL::DatabaseConnectionPtr connection = getConnection();
    IceSQL::TransactionHolder txn(connection);

    const_cast<SqlLLUPtr&>(llu) = new SqlLLU(connection, tablePrefix + "_LLU");
    const_cast<SqlSubscriberMapPtr&>(subscriberMap) =
        new SqlSubscriberMap(connection, tablePrefix + "_Subscribers", _communicator);

    txn.commit();
}

#else

DatabaseCache::DatabaseCache(const InstancePtr& instance) :
    _communicator(instance->communicator()),
    _envName(instance->serviceName()),
    _connection(Freeze::createConnection(_communicator, _envName))
{
}

DatabaseConnectionPtr
DatabaseCache::getConnection()
{
    return _connection;
}

DatabaseConnectionPtr
DatabaseCache::newConnection()
{
    return Freeze::createConnection(_communicator, _envName);
}


#endif
