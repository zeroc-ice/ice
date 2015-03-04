// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/Initialize.h>
#include <IceGrid/SqlDB/SqlStringApplicationInfoDict.h>
#include <IceDB/SqlTypes.h>
#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

#include <iomanip>

using namespace SqlDB;
using namespace IceGrid;
using namespace std;

namespace
{

string
escapePsqlBinary(const vector<Ice::Byte>& bytes)
{
    ostringstream result;
    for(unsigned int i = 0; i < bytes.size(); ++i)
    {
        result << "\\\\";
        result << setfill('0') << setw(3) << oct << (int)bytes[i];
    }
    return result.str();
}

string
escapeOdbcBinary(const vector<Ice::Byte>& bytes)
{
    ostringstream result;
    for(unsigned int i = 0; i < bytes.size(); ++i)
    {
        result << setfill('0') << setw(2) << hex << (int)bytes[i];
    }
    return result.str();
}

};

SqlStringApplicationInfoDict::SqlStringApplicationInfoDict(const DatabaseConnectionPtr& connection,
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
        queryString += " (name ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "VARCHAR(255) PRIMARY KEY, uuid VARCHAR(40), createTime BIGINT, createUser ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "TEXT, updateTime BIGINT, updateUser ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "TEXT, revision INTEGER, descriptor ";
        if(connection->sqlConnection().driverName() == "QPSQL")
        {
            queryString += "BYTEA";
        }
        else if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "VARBINARY(MAX)";
        }
        else
        {
            queryString += "BLOB";
        }
        queryString += ");";

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }
    }
}

void 
SqlStringApplicationInfoDict::put(const DatabaseConnectionPtr& connection,
                                  const string& name,
                                  const ApplicationInfo& info)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(_communicator);
    IceInternal::BasicStream stream(instance.get(), connection->getEncoding());
    stream.startWriteEncaps();
    stream.write(info.descriptor);
    stream.writePendingObjects();
    stream.endWriteEncaps();

    QSqlQuery query(connection->sqlConnection());
    ostringstream queryString;
    queryString << "UPDATE " << _table << " SET uuid = ?, createTime = '" << info.createTime 
                << "', createUser = ?, updateTime = '" << info.updateTime << "', updateUser = ?, revision = '" 
                << info.revision << "', descriptor = ";

    QString driver = connection->sqlConnection().driverName();
    if(driver == "QPSQL" || driver == "QODBC")
    {
        vector<Ice::Byte> bytes(stream.b.size());
        ::memcpy(&bytes[0], stream.b.begin(), stream.b.size());

        if(driver == "QPSQL")
        {
            queryString << "E'" << escapePsqlBinary(bytes) << "'";
        }
        else
        {
            queryString << "0x" << escapeOdbcBinary(bytes);
        }
    }
    else
    {
        queryString << "?";
    }
    queryString << " WHERE name = ?;";

    query.prepare(queryString.str().c_str());

    int placeholder = 0;
    query.bindValue(placeholder++, info.uuid.c_str());
    query.bindValue(placeholder++, info.createUser.c_str());
    query.bindValue(placeholder++, info.updateUser.c_str());

    if(driver != "QPSQL" && driver != "QODBC")
    {
        QByteArray bytes;
        bytes.resize(static_cast<int>(stream.b.size()));
        ::memcpy(bytes.data(), stream.b.begin(), stream.b.size());
        QVariant descriptor(bytes);

        query.bindValue(placeholder++, descriptor);
    }
    query.bindValue(placeholder, name.c_str());

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
            find(connection, name);
        }
        catch(const NotFoundException&)
        {
            QSqlQuery insertQuery(connection->sqlConnection());
            queryString.str("");
            queryString << "INSERT INTO " << _table << " VALUES(?, ?, '" << info.createTime << "', ?, '"
                        << info.updateTime << "', ?, '" << info.revision << "', ";

            if(driver == "QPSQL" || driver == "QODBC")
            {
                vector<Ice::Byte> bytes(stream.b.size());
                ::memcpy(&bytes[0], stream.b.begin(), stream.b.size());
 
                if(driver == "QPSQL")
                {
                    queryString << "E'" << escapePsqlBinary(bytes) << "'";
                }
                else
                {
                    queryString << "0x" << escapeOdbcBinary(bytes);
                }
            }
            else
            {
                queryString << "?";
            }
            queryString << ");";
 
            insertQuery.prepare(queryString.str().c_str());
            insertQuery.bindValue(0, name.c_str());
            insertQuery.bindValue(1, info.uuid.c_str());
            insertQuery.bindValue(2, info.createUser.c_str());
            insertQuery.bindValue(3, info.updateUser.c_str());
 
            if(driver != "QPSQL" && driver != "QODBC")
            {
                QByteArray bytes;
                bytes.resize(static_cast<int>(stream.b.size()));
                ::memcpy(bytes.data(), stream.b.begin(), stream.b.size());
                QVariant descriptor(bytes);
 
                insertQuery.bindValue(4, descriptor);
            }
 
            if(!insertQuery.exec())
            {
                throwDatabaseException(__FILE__, __LINE__, insertQuery.lastError());
            }
        }
    }
}

IceGrid::ApplicationInfo 
SqlStringApplicationInfoDict::find(const DatabaseConnectionPtr& connection,
                                   const string& name)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += " WHERE name = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, name.c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    if(query.next())
    {
        IceGrid::ApplicationInfo info;
        info.uuid = query.value(1).toString().toUtf8().data();
        info.createTime = query.value(2).toLongLong();
        info.createUser = query.value(3).toString().toUtf8().data();
        info.updateTime = query.value(4).toLongLong();
        info.updateUser = query.value(5).toString().toUtf8().data();
        info.revision = query.value(6).toInt();

        QByteArray bytes = query.value(7).toByteArray();

        IceInternal::InstancePtr instance = IceInternal::getInstance(_communicator);
        IceInternal::BasicStream stream(instance.get(), connection->getEncoding());
        stream.b.resize(bytes.size());
        ::memcpy(&stream.b[0], bytes.data(), bytes.size());
        stream.i = stream.b.begin();
        stream.startReadEncaps();
        stream.read(info.descriptor);
        stream.readPendingObjects();
        stream.endReadEncaps();

        return info;
    }
    else
    {
       throw NotFoundException(__FILE__, __LINE__);
    }
}

void
SqlStringApplicationInfoDict::getMap(const DatabaseConnectionPtr& connection,
                                     StringApplicationInfoDict& applicationMap)
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
        string name = query.value(0).toString().toUtf8().data();

        IceGrid::ApplicationInfo info;
        info.uuid = query.value(1).toString().toUtf8().data();
        info.createTime = query.value(2).toLongLong();
        info.createUser = query.value(3).toString().toUtf8().data();
        info.updateTime = query.value(4).toLongLong();
        info.updateUser = query.value(5).toString().toUtf8().data();
        info.revision = query.value(6).toInt();

        QByteArray bytes = query.value(7).toByteArray();

        IceInternal::InstancePtr instance = IceInternal::getInstance(_communicator);
        IceInternal::BasicStream stream(instance.get(), connection->getEncoding());
        stream.b.resize(bytes.size());
        ::memcpy(&stream.b[0], bytes.data(), bytes.size());
        stream.i = stream.b.begin();
        stream.startReadEncaps();
        stream.read(info.descriptor);
        stream.readPendingObjects();
        stream.endReadEncaps();

        applicationMap[name] = info;
    }
}

void 
SqlStringApplicationInfoDict::erase(const DatabaseConnectionPtr& connection, 
                                    const string& name)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += " WHERE name = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, name.c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

void
SqlStringApplicationInfoDict::clear(const DatabaseConnectionPtr& connection)
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
