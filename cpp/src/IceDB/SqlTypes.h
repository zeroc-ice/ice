// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_TYPES_H
#define SQL_TYPES_H

#include <IceDB/IceDB.h>

#include <IceUtil/Handle.h>
#include <IceUtil/Thread.h>

#include <Ice/CommunicatorF.h>
#include <Ice/Initialize.h> // For ThreadHook

#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

namespace SqlDB
{

//
// Generic expcetion for database failures.
//
class DatabaseException : public IceDB::DatabaseException
{
public:

    DatabaseException(const char*, int, const QSqlError&);
    virtual ~DatabaseException() throw();

    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    QSqlError error;
};

class DeadlockException : public IceDB::DeadlockException
{
public:

    DeadlockException(const char*, int, const QSqlError&);
    virtual ~DeadlockException() throw();

    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    QSqlError error;
};

class NotFoundException : public IceDB::NotFoundException
{
public:

    NotFoundException(const char*, int);
    virtual ~NotFoundException() throw();

    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

void throwDatabaseException(const char*, int, const QSqlError&);

//
// Database connection
//
class DatabaseConnection : public IceDB::DatabaseConnection
{
public:

    DatabaseConnection(const QSqlDatabase&, const QString&);

    virtual void beginTransaction();
    virtual void commitTransaction();
    virtual void rollbackTransaction();

    QSqlDatabase sqlConnection()
    {
        return _connection;
    }

    QString sqlConnectionName()
    {
        return _connectionName;
    }

private:

    const QSqlDatabase _connection;
    const QString _connectionName;
};
typedef IceUtil::Handle<DatabaseConnection> DatabaseConnectionPtr;


//
// Cache per thread of database information
//

class DatabaseCache : public IceUtil::Mutex, virtual public IceDB::DatabaseCache
{
public:
    
    IceDB::DatabaseConnectionPtr getConnection();
    IceDB::DatabaseConnectionPtr newConnection();

    void threadStopped();

protected:

    DatabaseCache(const Ice::CommunicatorPtr&, const std::string&, const std::string&, const std::string&, 
                  const std::string&, const std::string&, bool);
    virtual ~DatabaseCache();
                  
    typedef std::map<IceUtil::ThreadControl::ID, DatabaseConnectionPtr> ThreadDatabaseMap;

    QSqlDatabase _connection;
    ThreadDatabaseMap _cache;
};

typedef IceUtil::Handle<DatabaseCache> DatabaseCachePtr;

class ThreadHook : public Ice::ThreadNotification, public IceUtil::Mutex
{
public:

    ThreadHook();
    void setDatabaseCache(const DatabaseCachePtr&);

    virtual void start();
    virtual void stop();

private:

    DatabaseCachePtr _cache;
};

typedef IceUtil::Handle<ThreadHook> ThreadHookPtr;

template<class Table, class Key, class Value> class Wrapper : virtual public IceDB::Wrapper<Key, Value>
{
    typedef IceUtil::Handle<Table> TablePtr;

public:

    virtual std::map<Key, Value> getMap()
    {
        std::map<Key, Value> m;
        _table->getMap(_connection, m);
        return m;
    }

    virtual void
    put(const Key& key, const Value& data)
    {
        _table->put(_connection, key, data);
    }

    virtual Value
    find(const Key& key)
    {
        return _table->find(_connection, key);
    }

    virtual void
    erase(const Key& key)
    {
        _table->erase(_connection, key);
    }

    void
    clear()
    {
        _table->clear(_connection);
    }

    Wrapper(const DatabaseConnectionPtr& connection, const TablePtr& table) :
        _table(table),
        _connection(connection)
    {
    }

protected:

    const TablePtr _table;
    const DatabaseConnectionPtr _connection;
};

}

#endif
