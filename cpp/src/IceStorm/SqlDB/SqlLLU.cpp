// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceStorm/SqlDB/SqlLLU.h>

#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

using namespace IceStorm;
using namespace SqlDB;
using namespace std;

SqlLLU::SqlLLU(const DatabaseConnectionPtr& connection, const string& table) :
    _table(table)
{
    QStringList tables = connection->sqlConnection().tables(QSql::Tables);
    if(!tables.contains(_table.c_str(), Qt::CaseInsensitive))
    {
        QSqlQuery query(connection->sqlConnection());
        string queryString = "CREATE TABLE ";
        queryString += _table;
        queryString += " (id VARCHAR(255) PRIMARY KEY, generation INT, iteration INT);";

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }

        //
        // LLU table only ever contains one entry so we add it now.
        //
        QSqlQuery addQuery(connection->sqlConnection());
        queryString = "INSERT INTO ";
        queryString += _table;
        queryString += " VALUES('_master', '0', '0');";

        if(!addQuery.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, addQuery.lastError());
        }
    }
}

void 
SqlLLU::put(const DatabaseConnectionPtr& connection, const IceStormElection::LogUpdate& update)
{
    QSqlQuery query(connection->sqlConnection());
    ostringstream queryString;
    queryString << "UPDATE " << _table << " SET generation=" << update.generation << ", iteration=" 
                << update.iteration << " WHERE id='_master';";

    if(!query.exec(queryString.str().c_str()))
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

IceStormElection::LogUpdate 
SqlLLU::get(const DatabaseConnectionPtr& connection) const
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += " WHERE id = '_master';";

    if(!query.exec(queryString.c_str()))
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    if(query.next())
    {
        IceStormElection::LogUpdate update;
        update.generation = query.value(1).toInt();
        update.iteration = query.value(2).toInt();
        return update;
    }
    else
    {
       throw NotFoundException(__FILE__, __LINE__);
    }
}
