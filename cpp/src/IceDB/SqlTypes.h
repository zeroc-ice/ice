// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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
#include <IceUtil/FileUtil.h>

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
    virtual DatabaseException* ice_clone() const;
    virtual void ice_throw() const;

private:
    QSqlError error;
};

class DeadlockException : public IceDB::DeadlockException
{
public:

    DeadlockException(const char*, int, const QSqlError&);
    virtual ~DeadlockException() throw();

    virtual void ice_print(::std::ostream&) const;
    virtual DeadlockException* ice_clone() const;
    virtual void ice_throw() const;

private:
    QSqlError error;
};

class NotFoundException : public IceDB::NotFoundException
{
public:

    NotFoundException(const char*, int);
    virtual ~NotFoundException() throw();

    virtual NotFoundException* ice_clone() const;
    virtual void ice_throw() const;
};

void throwDatabaseException(const char*, int, const QSqlError&);

//
// Database connection
//
class DatabaseConnection : public virtual IceDB::DatabaseConnection
{
public:

    DatabaseConnection(const QSqlDatabase&, const QString&, const Ice::EncodingVersion&);

    virtual Ice::EncodingVersion getEncoding() const;

    virtual void beginTransaction();
    virtual void commitTransaction();
    virtual void rollbackTransaction();

    QSqlDatabase sqlConnection() const
    {
        return _connection;
    }

    QString sqlConnectionName() const
    {
        return _connectionName;
    }

private:

    const QSqlDatabase _connection;
    const QString _connectionName;
    const Ice::EncodingVersion _encoding;
};
typedef IceUtil::Handle<DatabaseConnection> DatabaseConnectionPtr;


//
// Connection pool
//

class ConnectionPool : public virtual IceDB::ConnectionPool
{
public:
    
    IceDB::DatabaseConnectionPtr getConnection();
    IceDB::DatabaseConnectionPtr newConnection();

    void threadStopped();

protected:

    ConnectionPool(const Ice::CommunicatorPtr&, const std::string&, const std::string&, const std::string&, int,
                  const std::string&, const std::string&, bool, const Ice::EncodingVersion&);
    virtual ~ConnectionPool();
                  
    typedef std::map<IceUtil::ThreadControl::ID, DatabaseConnectionPtr> ThreadDatabaseMap;

    QSqlDatabase _connection;
    ThreadDatabaseMap _cache;
    
private:
    
    IceUtilInternal::FileLockPtr _fileLock;
    const Ice::EncodingVersion _encoding;
    IceUtil::Mutex _mutex;
};

typedef IceUtil::Handle<ConnectionPool> ConnectionPoolPtr;

class ThreadHook : public Ice::ThreadNotification
{
public:

    ThreadHook();
    void setConnectionPool(const ConnectionPoolPtr&);

    virtual void start();
    virtual void stop();

private:

    ConnectionPoolPtr _cache;
    IceUtil::Mutex _mutex;
};

typedef IceUtil::Handle<ThreadHook> ThreadHookPtr;

template<class Table, class Key, class Value> class Wrapper : public virtual IceDB::Wrapper<Key, Value>
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
