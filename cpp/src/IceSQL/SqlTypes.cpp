// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/Instance.h>
#include <Ice/LoggerUtil.h>
#include <IceUtil/UUID.h>
#include <IceUtil/StringUtil.h>
#include <IceSQL/SqlTypes.h>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlDriver>

using namespace IceSQL;
using namespace std;

DatabaseException::DatabaseException(const char* file, int line) :
    ::IceUtil::Exception(file, line)
{
}

DatabaseException::DatabaseException(const char* file, int line, const QSqlError& err) :
    ::IceUtil::Exception(file, line),
    error(err)
{
}

DatabaseException::~DatabaseException() throw()
{
}

::std::string
DatabaseException::ice_name() const
{
    return "IceSQL::DatabaseException";
}

void
DatabaseException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\n  database error: " << error.databaseText().toUtf8().data();
    out << "\n  driver error: " << error.driverText().toUtf8().data();
}

::IceUtil::Exception*
DatabaseException::ice_clone() const
{
    return new DatabaseException(*this);
}

void
DatabaseException::ice_throw() const
{
    throw *this;
}

DeadlockException::DeadlockException(const char* file, int line) :
    DatabaseException(file, line)
{
}

DeadlockException::DeadlockException(const char* file, int line, const QSqlError& err) :
    DatabaseException(file, line, err)
{
}

DeadlockException::~DeadlockException() throw()
{
}

::std::string
DeadlockException::ice_name() const
{
    return "IceSQL::DeadlockException";
}

void
DeadlockException::ice_print(ostream& out) const
{
    DatabaseException::ice_print(out);
}

::IceUtil::Exception*
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
    DatabaseException(file, line)
{
}

NotFoundException::~NotFoundException() throw()
{
}

::std::string
NotFoundException::ice_name() const
{
    return "IceSQL::NotFoundException";
}

void
NotFoundException::ice_print(ostream& out) const
{
    IceUtil::Exception::ice_print(out);
}

::IceUtil::Exception*
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
IceSQL::throwDatabaseException(const char* file, int line, const QSqlError& err)
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

TransactionHolder::TransactionHolder(const DatabaseConnectionPtr& connection) :
    _connection(connection->connection),
    _committed(false)
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

TransactionHolder::~TransactionHolder()
{
    if(!_committed)
    {
        if(!const_cast<QSqlDatabase&>(_connection).rollback())
        {
            throwDatabaseException(__FILE__, __LINE__, _connection.lastError());
        }
    }
}

void
TransactionHolder::commit()
{
    if(!const_cast<QSqlDatabase&>(_connection).commit())
    {
        throwDatabaseException(__FILE__, __LINE__, _connection.lastError());
    }
    _committed = true;
}

DatabaseCache::DatabaseCache(const Ice::CommunicatorPtr& communicator, 
                             const string& type,
                             const string& name,
                             const string& host,
                             const string& user,
                             const string& password,
                             bool requiresBlob) :
    _communicator(communicator)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    _connection = QSqlDatabase::addDatabase(type.c_str(), IceUtil::generateUUID().c_str());
    _connection.setDatabaseName(name.c_str());
    _connection.setHostName(host.c_str());
    _connection.setUserName(user.c_str());
    _connection.setPassword(password.c_str());

    DatabaseConnectionPtr connection = getConnection();
    QSqlDriver* driver = connection->connection.driver();
    if(!driver->hasFeature(QSqlDriver::Transactions))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "SQL database driver requires transaction support");
    }
    if(!driver->hasFeature(QSqlDriver::Unicode))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "SQL database driver requires unicode support");
    }
    if(requiresBlob && connection->connection.driverName() != "QODBC" && !driver->hasFeature(QSqlDriver::BLOB))
    {
        throw Ice::InitializationException(__FILE__, __LINE__, "SQL database driver requires blob support");
    }

    ThreadHookPtr threadHook = 
        ThreadHookPtr::dynamicCast(IceInternal::getInstance(communicator)->initializationData().threadHook);
    if(!threadHook)
    {
        Ice::Warning out(communicator->getLogger());
        out << "Thread notification hook for SQL database usage has not been configured";
    }
    else
    {
        threadHook->setDatabaseCache(this);
    }
}

DatabaseCache::~DatabaseCache()
{
    //
    // QSqlDatabase references must be removed before calling removeDatabase.
    //
    vector<QString> names;
    for(ThreadDatabaseMap::iterator p = _cache.begin(); p != _cache.end(); ++p)
    {
        names.push_back(p->second->connectionName);
    }
    _cache.clear();

    for(unsigned int i = 0; i < names.size(); ++i)
    {
        QSqlDatabase::removeDatabase(names[i]);
    }
}

DatabaseConnectionPtr
DatabaseCache::getConnection()
{
    IceUtil::Mutex::Lock lock(*this);
    
    ThreadDatabaseMap::iterator p = _cache.find(IceUtil::ThreadControl().id());
    if(p != _cache.end())
    {
        //
        // We have a cached connection. Perform a simple query to make sure 
        // connection has not been lost.
        //
        {
            QSqlQuery query(p->second->connection);
            if(query.exec("SELECT 1"))
            {
                return p->second;
            }
        }

        //
        // QSqlDatabase reference must be removed before calling removeDatabase.
        //
        QString connectionName = p->second->connectionName;
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

    DatabaseConnectionPtr db = new DatabaseConnection(connection, connectionName);
    _cache[IceUtil::ThreadControl().id()] = db;
    return db;
}

DatabaseConnectionPtr
DatabaseCache::newConnection()
{
    return getConnection();
}

void
DatabaseCache::threadStopped()
{
    IceUtil::Mutex::Lock lock(*this);

    ThreadDatabaseMap::iterator p = _cache.find(IceUtil::ThreadControl().id());
    if(p != _cache.end())
    {
        //
        // QSqlDatabase reference must be removed before calling removeDatabase.
        //
        QString connectionName = p->second->connectionName;
        _cache.erase(p);
        QSqlDatabase::removeDatabase(connectionName);
    }
}

ThreadHook::ThreadHook()
{
}

void
ThreadHook::setDatabaseCache(const DatabaseCachePtr& cache)
{
    IceUtil::Mutex::Lock sync(*this);

    _cache = cache;
}

void
ThreadHook::start()
{
}

void
ThreadHook::stop()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_cache)
    {
        _cache->threadStopped();
    }
}

extern "C"
{

ICE_DECLSPEC_EXPORT ::Ice::Plugin*
createThreadHook(const Ice::CommunicatorPtr& communicator, const string& name, const Ice::StringSeq& args)
{
    return new Ice::ThreadHookPlugin(communicator, new ThreadHook);
}

}


