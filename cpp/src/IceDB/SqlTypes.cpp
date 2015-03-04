// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <IceUtil/UUID.h>
#include <IceUtil/StringUtil.h>
#include <IceDB/SqlTypes.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>

using namespace SqlDB;
using namespace std;

DatabaseException::DatabaseException(const char* file, int line, const QSqlError& err) :
    IceDB::DatabaseException(file, line),
    error(err)
{
}

DatabaseException::~DatabaseException() throw()
{
}

void
DatabaseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n  database error: " << error.databaseText().toUtf8().data();
    out << "\n  driver error: " << error.driverText().toUtf8().data();
}

DatabaseException*
DatabaseException::ice_clone() const
{
    return new DatabaseException(*this);
}

void
DatabaseException::ice_throw() const
{
    throw *this;
}

DeadlockException::DeadlockException(const char* file, int line, const QSqlError& err) :
    IceDB::DeadlockException(file, line),
    error(err)
{
}

DeadlockException::~DeadlockException() throw()
{
}

void
DeadlockException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n  database error: " << error.databaseText().toUtf8().data();
    out << "\n  driver error: " << error.driverText().toUtf8().data();
}

DeadlockException*
DeadlockException::ice_clone() const
{
    return new DeadlockException(*this);
}

void
DeadlockException::ice_throw() const
{
    throw *this;
}

NotFoundException::NotFoundException(const char* file, int line) :
    IceDB::NotFoundException(file, line)
{
}

NotFoundException::~NotFoundException() throw()
{
}

NotFoundException*
NotFoundException::ice_clone() const
{
    return new NotFoundException(*this);
}

void
NotFoundException::ice_throw() const
{
    throw *this;
}

void
SqlDB::throwDatabaseException(const char* file, int line, const QSqlError& err)
{
    string s = IceUtilInternal::toLower(err.databaseText().toUtf8().data());
    if(s.find("deadlock") != string::npos || s.find("database is locked") != string::npos ||
       s.find("could not serialize") != string::npos)
    {
        throw DeadlockException(file, line, err);
    }
    else
    {
        throw DatabaseException(file, line, err);
    }
}

DatabaseConnection::DatabaseConnection(const QSqlDatabase& c, const QString& cn, const Ice::EncodingVersion& encoding) :
    _connection(c),
    _connectionName(cn),
    _encoding(encoding)
{
}

Ice::EncodingVersion
DatabaseConnection::getEncoding() const
{
    return _encoding;
}

void
DatabaseConnection::beginTransaction()
{
    if(_connection.driverName() == "QSQLITE")
    {
        QSqlQuery query(_connection);
        string queryString = "BEGIN EXCLUSIVE;";

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }
    }
    else
    {
        if(!const_cast<QSqlDatabase&>(_connection).transaction())
        {
            throwDatabaseException(__FILE__, __LINE__, _connection.lastError());
        }
    }
}

void
DatabaseConnection::commitTransaction()
{
    if(!const_cast<QSqlDatabase&>(_connection).commit())
    {
        throwDatabaseException(__FILE__, __LINE__, _connection.lastError());
    }
}

void
DatabaseConnection::rollbackTransaction()
{
    if(!const_cast<QSqlDatabase&>(_connection).rollback())
    {
        throwDatabaseException(__FILE__, __LINE__, _connection.lastError());
    }
}

ConnectionPool::ConnectionPool(const Ice::CommunicatorPtr& communicator, 
                             const string& type,
                             const string& name,
                             const string& host,
                             int port,
                             const string& user,
                             const string& password,
                             bool requiresBlob,
                             const Ice::EncodingVersion& encoding) : _encoding(encoding)
{
    //
    // File lock to prevent multiple process open the same db env.
    //
    if(type == "QSQLITE")
    {
        _fileLock = new IceUtilInternal::FileLock(name + ".lock");
    }

    _connection = QSqlDatabase::addDatabase(type.c_str(), IceUtil::generateUUID().c_str());
    _connection.setDatabaseName(name.c_str());
    _connection.setHostName(host.c_str());
    if(port != 0)
    {
        _connection.setPort(port);
    }
    _connection.setUserName(user.c_str());
    _connection.setPassword(password.c_str());

    DatabaseConnectionPtr connection = DatabaseConnectionPtr::dynamicCast(getConnection());
    QSqlDriver* driver = connection->sqlConnection().driver();
    if(!driver->hasFeature(QSqlDriver::Transactions))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "SQL database driver requires transaction support");
    }
    if(!driver->hasFeature(QSqlDriver::Unicode))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "SQL database driver requires unicode support");
    }
    if(requiresBlob && connection->sqlConnection().driverName() != "QODBC" && !driver->hasFeature(QSqlDriver::BLOB))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "SQL database driver requires blob support");
    }
}

ConnectionPool::~ConnectionPool()
{
    //
    // QSqlDatabase references must be removed before calling removeDatabase.
    //
    vector<QString> names;
    for(ThreadDatabaseMap::iterator p = _cache.begin(); p != _cache.end(); ++p)
    {
        names.push_back(p->second->sqlConnectionName());
    }
    _cache.clear();

    for(unsigned int i = 0; i < names.size(); ++i)
    {
        QSqlDatabase::removeDatabase(names[i]);
    }
}

IceDB::DatabaseConnectionPtr
ConnectionPool::getConnection()
{
    IceUtil::Mutex::Lock lock(_mutex);
    
    ThreadDatabaseMap::iterator p = _cache.find(IceUtil::ThreadControl().id());
    if(p != _cache.end())
    {
        //
        // We have a cached connection. Perform a simple query to make sure 
        // connection has not been lost.
        //
        {
            QSqlQuery query(p->second->sqlConnection());
            if(query.exec("SELECT 1"))
            {
                return p->second;
            }
        }

        //
        // QSqlDatabase reference must be removed before calling removeDatabase.
        //
        QString connectionName = p->second->sqlConnectionName();
        _cache.erase(p);
        QSqlDatabase::removeDatabase(connectionName);
    }

    //
    // Create a new connection
    //
    QString connectionName = IceUtil::generateUUID().c_str();
    QSqlDatabase connection = QSqlDatabase::cloneDatabase(_connection, connectionName);
    if(!connection.open())
    {
        throwDatabaseException(__FILE__, __LINE__, connection.lastError());
    }

    //
    // Set isolation level, for SQLite this is done on a per transaction level.
    //
    if(connection.driverName() != "QSQLITE")
    {
        QSqlQuery query(connection);
        string queryString;
        if(connection.driverName() == "QPSQL")
        {
            queryString = "SET SESSION CHARACTERISTICS AS TRANSACTION ISOLATION LEVEL SERIALIZABLE";
        }
        else if(connection.driverName() == "QMYSQL")
        {
            queryString = "SET SESSION TRANSACTION ISOLATION LEVEL SERIALIZABLE;";
        }
        else if(connection.driverName() == "QODBC")
        {
            queryString = "SET TRANSACTION ISOLATION LEVEL SERIALIZABLE;";
        }

        if(!query.exec(queryString.c_str()))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }
    }

    //
    // Set storage engine for MySQL.
    //
    if(connection.driverName() == "QMYSQL")
    {
        QSqlQuery query(connection);
        if(!query.exec("SET storage_engine=INNODB;"))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }
    }

    //
    // Set UTF-8 character set for MySQL and PostgreSQL.
    //
    if(connection.driverName() == "QMYSQL" || connection.driverName() == "QPSQL")
    {
        QSqlQuery query(connection);
        if(!query.exec("SET NAMES 'UTF8';"))
        {
            throwDatabaseException(__FILE__, __LINE__, query.lastError());
        }
    }

    DatabaseConnectionPtr db = new DatabaseConnection(connection, connectionName, _encoding);
    _cache[IceUtil::ThreadControl().id()] = db;
    return db;
}

IceDB::DatabaseConnectionPtr
ConnectionPool::newConnection()
{
    return getConnection();
}

void
ConnectionPool::threadStopped()
{
    IceUtil::Mutex::Lock lock(_mutex);

    ThreadDatabaseMap::iterator p = _cache.find(IceUtil::ThreadControl().id());
    if(p != _cache.end())
    {
        //
        // QSqlDatabase reference must be removed before calling removeDatabase.
        //
        QString connectionName = p->second->sqlConnectionName();
        _cache.erase(p);
        QSqlDatabase::removeDatabase(connectionName);
    }
}

ThreadHook::ThreadHook()
{
}

void
ThreadHook::setConnectionPool(const ConnectionPoolPtr& cache)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _cache = cache;
}

void
ThreadHook::start()
{
}

void
ThreadHook::stop()
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(_cache)
    {
        _cache->threadStopped();
    }
}
