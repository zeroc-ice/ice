// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <IceStorm/SqlDB/SqlSubscriberMap.h>
#include <QtSql/QSqlQuery>
#include <QtCore/QVariant>
#include <QtCore/QStringList>

using namespace IceStorm;
using namespace SqlDB;
using namespace std;

SqlSubscriberMap::QoSMap::QoSMap(const DatabaseConnectionPtr& connection,
                                 const string& table, 
                                 const Ice::CommunicatorPtr& communicator) :
    _table(table),
    _communicator(communicator)
{
    QStringList tables = connection->sqlConnection().tables(QSql::Tables);
    if(!tables.contains(_table.c_str(), Qt::CaseInsensitive))
    {
        QSqlQuery query(connection->sqlConnection());
        string queryString = "CREATE TABLE ";
        queryString += _table;
        queryString += " (topic VARCHAR(255), id VARCHAR(255), name ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "TEXT, value ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "TEXT);";

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }

        QSqlQuery idx1Query(connection->sqlConnection());
        queryString = "CREATE INDEX IDX_";
        queryString += _table;
        queryString += "_TOPIC_SUBSCRIBER ON ";
        queryString += _table;
        queryString += " (topic, id);";

        if(!idx1Query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, idx1Query.lastError());
        }

        QSqlQuery idx2Query(connection->sqlConnection());
        queryString = "CREATE INDEX IDX_";
        queryString += _table;
        queryString += "_TOPIC ON ";
        queryString += _table;
        queryString += " (topic);";

        if(!idx2Query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, idx2Query.lastError());
        }
    }
}

void
SqlSubscriberMap::QoSMap::put(const DatabaseConnectionPtr& connection,
                              const SubscriberRecordKey& key,
                              const QoS& qos)
{
    erase(connection, key);

    for(QoS::const_iterator p = qos.begin(); p != qos.end(); ++p)
    {
        QSqlQuery query(connection->sqlConnection());
        string queryString = "INSERT INTO ";
        queryString += _table;
        queryString += " VALUES(?, ?, ?, ?)";
        query.prepare(queryString.c_str());
        query.bindValue(0, _communicator->identityToString(key.topic).c_str());
        query.bindValue(1, _communicator->identityToString(key.id).c_str());
        query.bindValue(2, p->first.c_str());
        query.bindValue(3, p->second.c_str());

        if(!query.exec())
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }
    }
}

QoS
SqlSubscriberMap::QoSMap::find(const DatabaseConnectionPtr& connection,
                               const SubscriberRecordKey& key)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += " WHERE topic = ? AND id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->identityToString(key.topic).c_str());
    query.bindValue(1, _communicator->identityToString(key.id).c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    QoS qos;
    while(query.next())
    {
        qos[query.value(2).toString().toUtf8().data()] = query.value(3).toString().toUtf8().data();
    }
    return qos;
}

void
SqlSubscriberMap::QoSMap::erase(const DatabaseConnectionPtr& connection,
                                const SubscriberRecordKey& key)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += " WHERE topic = ? AND id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->identityToString(key.topic).c_str());
    query.bindValue(1, _communicator->identityToString(key.id).c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

void
SqlSubscriberMap::QoSMap::eraseTopic(const DatabaseConnectionPtr& connection,
                                     const Ice::Identity& topic)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += " WHERE topic = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->identityToString(topic).c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

void
SqlSubscriberMap::QoSMap::clear(const DatabaseConnectionPtr& connection)
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

SqlSubscriberMap::SqlSubscriberMap(const DatabaseConnectionPtr& connection,
                                   const string& table,
                                   const Ice::CommunicatorPtr& communicator) :
    _table(table),
    _communicator(communicator),
    _qosMap(connection, table + "_QoS", communicator)
{
    QStringList tables = connection->sqlConnection().tables(QSql::Tables);
    if(!tables.contains(_table.c_str(), Qt::CaseInsensitive))
    {
        QSqlQuery query(connection->sqlConnection());
        string queryString = "CREATE TABLE ";
        queryString += _table;
        queryString += " (topic VARCHAR(255), id VARCHAR(255), topicName ";
        if(connection->sqlConnection().driverName() == "QODBC")
        {
            queryString += "N";
        }
        queryString += "TEXT, link TEXT, obj TEXT, cost INT, theTopic TEXT, PRIMARY KEY (topic, id));";

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }

        QSqlQuery idxQuery(connection->sqlConnection());
        queryString = "CREATE INDEX IDX_";
        queryString += _table;
        queryString += "_TOPIC ON ";
        queryString += _table;
        queryString += " (topic);";

        if(!idxQuery.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, idxQuery.lastError());
        }
    }
}

void 
SqlSubscriberMap::put(const DatabaseConnectionPtr& connection,
                      const SubscriberRecordKey& key,
                      const SubscriberRecord& record)
{
    QString driver = connection->sqlConnection().driverName();

    QSqlQuery query(connection->sqlConnection());
    ostringstream queryString;
    queryString <<  "UPDATE " << _table 
                << " SET topicName = ?, link = '" << (record.link ? "true" : "false")
                << "', obj = ?, cost = '" << record.cost
                << "', theTopic = ? WHERE topic = ? AND id = ?;";

    query.prepare(queryString.str().c_str());
    query.bindValue(0, record.topicName.c_str());
    query.bindValue(1, _communicator->proxyToString(record.obj).c_str());
    query.bindValue(2, _communicator->proxyToString(record.theTopic).c_str());
    query.bindValue(3, _communicator->identityToString(key.topic).c_str());
    query.bindValue(4, _communicator->identityToString(key.id).c_str());

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
            find(connection, key);
        }
        catch(const NotFoundException&)
        {
            QSqlQuery insertQuery(connection->sqlConnection());
            queryString.str("");
            queryString << "INSERT INTO " << _table << " VALUES(?, ?, ?, '" << (record.link ? "true" : "false")
                        << "', ?, '" << record.cost << "', ?);";

            insertQuery.prepare(queryString.str().c_str());
            insertQuery.bindValue(0, _communicator->identityToString(key.topic).c_str());
            insertQuery.bindValue(1, _communicator->identityToString(key.id).c_str());
            insertQuery.bindValue(2, record.topicName.c_str());
            insertQuery.bindValue(3, _communicator->proxyToString(record.obj).c_str());
            insertQuery.bindValue(4, _communicator->proxyToString(record.theTopic).c_str());

            if(!insertQuery.exec())
            {
                throwDatabaseException(__FILE__, __LINE__, insertQuery.lastError());
            }
        }
    }

    _qosMap.put(connection, key, record.theQoS);
}

SubscriberRecord 
SqlSubscriberMap::find(const DatabaseConnectionPtr& connection,
                       const SubscriberRecordKey& key)
{
    QSqlQuery query(connection->sqlConnection());
    string queryString = "SELECT * FROM ";
    queryString += _table;
    queryString += " WHERE topic = ? AND id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->identityToString(key.topic).c_str());
    query.bindValue(1, _communicator->identityToString(key.id).c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }

    if(query.next())
    {
        SubscriberRecord record;
        record.topicName = query.value(2).toString().toUtf8().data();
        record.id = key.id;
        record.link = string(query.value(3).toString().toUtf8().data()) == "true" ? true : false;
        record.obj = _communicator->stringToProxy(query.value(4).toString().toUtf8().data());
        record.cost = query.value(5).toInt();
        record.theTopic = 
            TopicPrx::uncheckedCast(_communicator->stringToProxy(query.value(6).toString().toUtf8().data()));

        record.theQoS = _qosMap.find(connection, key);

        return record;
    }
    else
    {
       throw NotFoundException(__FILE__, __LINE__);
    }
}

void
SqlSubscriberMap::getMap(const DatabaseConnectionPtr& connection,
                         SubscriberMap& smap)
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
        SubscriberRecordKey key;
        key.topic = _communicator->stringToIdentity(query.value(0).toString().toUtf8().data());
        key.id = _communicator->stringToIdentity(query.value(1).toString().toUtf8().data());

        SubscriberRecord record;
        record.topicName = query.value(2).toString().toUtf8().data();
        record.id = key.id;
        record.link = string(query.value(3).toString().toUtf8().data()) == "true" ? true : false;
        record.obj = _communicator->stringToProxy(query.value(4).toString().toUtf8().data());
        record.cost = query.value(5).toInt();
        record.theTopic = 
            TopicPrx::uncheckedCast(_communicator->stringToProxy(query.value(6).toString().toUtf8().data()));

        record.theQoS = _qosMap.find(connection, key);

        smap[key] = record;
    }
}

void 
SqlSubscriberMap::erase(const DatabaseConnectionPtr& connection,
                        const SubscriberRecordKey& key)
{
    _qosMap.erase(connection, key);

    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += " WHERE topic = ? AND id = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->identityToString(key.topic).c_str());
    query.bindValue(1, _communicator->identityToString(key.id).c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

void 
SqlSubscriberMap::eraseTopic(const DatabaseConnectionPtr& connection,
                             const Ice::Identity& topic)
{
    _qosMap.eraseTopic(connection, topic);

    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += " WHERE topic = ?;";

    query.prepare(queryString.c_str());
    query.bindValue(0, _communicator->identityToString(topic).c_str());

    if(!query.exec())
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}

void
SqlSubscriberMap::clear(const DatabaseConnectionPtr& connection)
{
    _qosMap.clear(connection);

    QSqlQuery query(connection->sqlConnection());
    string queryString = "DELETE FROM ";
    queryString += _table;
    queryString += ";";

    if(!query.exec(queryString.c_str()))
    {
        throwDatabaseException(__FILE__, __LINE__, query.lastError());
    }
}
