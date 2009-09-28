// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/DatabaseCache.h>
#ifdef QTSQL
#  include <Ice/Communicator.h>
#endif

using namespace IceGrid;
using namespace std;

#ifdef QTSQL

DatabaseCache::DatabaseCache(const Ice::CommunicatorPtr& communicator, 
                             const string&,
                             const string& instanceName,
                             const string& replicaName) :
    IceSQL::DatabaseCache(communicator,
                          communicator->getProperties()->getProperty("IceGrid.SQL.DatabaseType"),
                          communicator->getProperties()->getProperty("IceGrid.SQL.DatabaseName"),
                          communicator->getProperties()->getProperty("IceGrid.SQL.HostName"),
                          communicator->getProperties()->getProperty("IceGrid.SQL.UserName"),
                          communicator->getProperties()->getProperty("IceGrid.SQL.Password"),
                          true)
{
    string tablePrefix = instanceName + "_" + replicaName;
    replace(tablePrefix.begin(), tablePrefix.end(), '.', '_');
    replace(tablePrefix.begin(), tablePrefix.end(), '-', '_');
    replace(tablePrefix.begin(), tablePrefix.end(), ' ', '_');
    replace(tablePrefix.begin(), tablePrefix.end(), ';', '_');

    IceSQL::DatabaseConnectionPtr connection = getConnection();
    IceSQL::TransactionHolder txn(connection);

    const_cast<SqlStringApplicationInfoDictPtr&>(applications) = 
        new SqlStringApplicationInfoDict(connection, tablePrefix + "_Applications", _communicator);
    const_cast<SqlStringAdapterInfoDictPtr&>(adapters) = 
        new SqlStringAdapterInfoDict(connection, tablePrefix + "_Adapters", _communicator);
    const_cast<SqlIdentityObjectInfoDictPtr&>(objects) =
        new SqlIdentityObjectInfoDict(connection, tablePrefix + "_Objects", _communicator);
    const_cast<SqlIdentityObjectInfoDictPtr&>(internalObjects) =
        new SqlIdentityObjectInfoDict(connection, tablePrefix + "_InternalObjects", _communicator);

    txn.commit();
}

#else

DatabaseCache::DatabaseCache(const Ice::CommunicatorPtr& communicator,
                             const string& envName,
                             const string&,
                             const string&) :
    _communicator(communicator),
    _envName(envName),
    _connection(Freeze::createConnection(communicator, envName))
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
