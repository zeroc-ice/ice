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

#include <IceUtil/Exception.h>
#include <IceUtil/Thread.h>
#include <Ice/CommunicatorF.h>
#include <Ice/Initialize.h>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

namespace IceSQL
{

//
// Generic expcetion for database failures.
//
class DatabaseException : public IceUtil::Exception
{
public:

    DatabaseException(const char*, int);
    DatabaseException(const char*, int, const QSqlError&);
    virtual ~DatabaseException() throw();

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;

    QSqlError error;
};

class DeadlockException : public DatabaseException
{
public:

    DeadlockException(const char*, int);
    DeadlockException(const char*, int, const QSqlError&);
    virtual ~DeadlockException() throw();

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

class NotFoundException : public DatabaseException
{
public:

    NotFoundException(const char*, int);
    virtual ~NotFoundException() throw();

    virtual ::std::string ice_name() const;
    virtual void ice_print(::std::ostream&) const;
    virtual ::IceUtil::Exception* ice_clone() const;
    virtual void ice_throw() const;
};

void throwDatabaseException(const char*, int, const QSqlError&);

//
// Database connection
//
class DatabaseConnection : public IceUtil::Shared
{
public:

    DatabaseConnection(const QSqlDatabase& c, const QString& cn) :
        connection(c),
        connectionName(cn)
    {
    }

    const QSqlDatabase connection;
    const QString connectionName;
};

typedef IceUtil::Handle<DatabaseConnection> DatabaseConnectionPtr;

//
// Transaction wrapper class
//
class TransactionHolder
{
public:

    TransactionHolder(const DatabaseConnectionPtr&);
    ~TransactionHolder();

    void commit();

private:

    const QSqlDatabase _connection;
    bool _committed;
};

//
// Cache per thread of database information
//

class DatabaseCache : public IceUtil::Mutex, public IceUtil::Shared
{
public:

    DatabaseConnectionPtr getConnection();
    DatabaseConnectionPtr newConnection();

    void threadStopped();

protected:

    DatabaseCache(const Ice::CommunicatorPtr&, const std::string&, const std::string&, const std::string&, 
                  const std::string&, const std::string&, bool);
    virtual ~DatabaseCache();
                  
    typedef std::map<IceUtil::ThreadControl::ID, DatabaseConnectionPtr> ThreadDatabaseMap;

    const Ice::CommunicatorPtr _communicator;
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

}

#endif
