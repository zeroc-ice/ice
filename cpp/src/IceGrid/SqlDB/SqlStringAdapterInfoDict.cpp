// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <IceGrid/SqlDB/SqlStringAdapterInfoDict.h>
#include <IceDB/SqlTypes.h>
#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

using namespace SqlDB;
using namespace IceGrid;
using namespace std;

SqlStringAdapterInfoDict::SqlStringAdapterInfoDict(const DatabaseConnectionPtr& connection,
                                                   const string& table,
                                                   const Ice::CommunicatorPtr& communicator) : 
    _communicator(communicator),
    _table(table)

{
    QStringList tables = connection->sqlConnection().tables(QSql::Tables);
    if(!tables.contains(_table.c_str(), Qt::CaseInsensitive))
    {
        QSqlQuery query(connection->sqlConnection());
        string queryString = "CREATE TABLE ";
        queryString += _table;
        queryString += " (id ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "VARCHAR(255) PRIMARY KEY, proxy TEXT, replicaGroupId ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "VARCHAR(255));";

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }

        QSqlQuery idxQuery(connection->sqlConnection());
        queryString = "CREATE INDEX IDX_";
        queryString += _table;
        queryString += "_REPLICA_GROUP_ID ON ";
        queryString += _table;
        queryString += " (replicaGroupId);";

        if(!idxQuery.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, idxQuery.lastError());
        }
    }
}

void 
SqlStringAdapterInfoDict::put(const DatabaseConnectionPtr& connection,
                              const string& id,
                              const AdapterInfo& info)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "UPDATE ";
    queryString += _table;
    queryString += " SET proxy = ?, replicaGroupId = ? WHERE id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->proxyToString(info.proxy).c_str());
    query.bindValue(1, info.replicaGroupId.c_str());
    query.bindValue(2, id.c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    if(query.numRowsAffected() == 0)
    {
        //
        // We do a find since some databases (MySQL) return 0 for number of rows affected
        // if row exists but data was not changed from previous values.
        //
        try
        {
            find(connection, id);
        }
        catch(const NotFoundException&)
        {
            QSqlQuery insertQuery(connection->sqlConnection());
            queryString = "INSERT INTO ";
            queryString += _table;
            queryString += " VALUES(?, ?, ?);";

            insertQuery.prepare(queryString.c_str());
            insertQuery.bindValue(0, id.c_str());
            insertQuery.bindValue(1, _communicator->proxyToString(info.proxy).c_str());
            insertQuery.bindValue(2, info.replicaGroupId.c_str());

            if(!insertQuery.exec())
            {
                throwDatabaseException(__FILE__, __LINE__, insertQuery.lastError());
            }
        }
    }
}

IceGrid::AdapterInfo 
SqlStringAdapterInfoDict::find(const DatabaseConnectionPtr& connection, 
                               const string& id)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += " WHERE id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, id.c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    if(query.next())
    {
        IceGrid::AdapterInfo info;
        info.id = query.value(0).toString().toUtf8().data();
        info.proxy = _communicator->stringToProxy(query.value(1).toString().toUtf8().data());
        info.replicaGroupId = query.value(2).toString().toUtf8().data();
        return info;
    }
    else
    {
       throw NotFoundException(__FILE__, __LINE__);
    }
}

vector<IceGrid::AdapterInfo> 
SqlStringAdapterInfoDict::findByReplicaGroupId(const DatabaseConnectionPtr& connection,
                                               const string& id)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += " WHERE replicaGroupId = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, id.c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    vector<IceGrid::AdapterInfo> infos;
    while(query.next())
    {
        IceGrid::AdapterInfo info;
        info.id = query.value(0).toString().toUtf8().data();
        info.proxy = _communicator->stringToProxy(query.value(1).toString().toUtf8().data());
        info.replicaGroupId = query.value(2).toString().toUtf8().data();
        infos.push_back(info);
    }
    return infos;
}

void
SqlStringAdapterInfoDict::getMap(const DatabaseConnectionPtr& connection,
                                 StringAdapterInfoDict& adapterMap)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += ";";

    if(!query.exec(queryString.c_str()))
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    while(query.next())
    {
        IceGrid::AdapterInfo info;
        info.id = query.value(0).toString().toUtf8().data();
        info.proxy = _communicator->stringToProxy(query.value(1).toString().toUtf8().data());
        info.replicaGroupId = query.value(2).toString().toUtf8().data();
        adapterMap[info.id] = info;
    }
}

void 
SqlStringAdapterInfoDict::erase(const DatabaseConnectionPtr& connection, 
                                const string& id)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += " WHERE id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, id.c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

void
SqlStringAdapterInfoDict::clear(const DatabaseConnectionPtr& connection)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += ";";

    if(!query.exec(queryString.c_str()))
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}
