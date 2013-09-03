// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_TYPES_H
#define FREEZE_TYPES_H

#include <IceDB/IceDB.h>
#include <Freeze/Freeze.h>

namespace FreezeDB
{

class DatabaseException : public IceDB::DatabaseException
{
public:

    DatabaseException(const char*, int, const Freeze::DatabaseException&);
    virtual ~DatabaseException() throw();

    virtual void ice_print(::std::ostream&) const;
    virtual DatabaseException* ice_clone() const;
    virtual void ice_throw() const;

private:
    std::string message;
};

class DeadlockException : public IceDB::DeadlockException
{
public:

    DeadlockException(const char*, int, const Freeze::DatabaseException&);
    virtual ~DeadlockException() throw();

    virtual void ice_print(::std::ostream&) const;
    virtual DeadlockException* ice_clone() const;
    virtual void ice_throw() const;

private:
    std::string message;
};

class NotFoundException : public IceDB::NotFoundException
{
public:

    NotFoundException(const char*, int);
    virtual ~NotFoundException() throw();

    virtual NotFoundException* ice_clone() const;
    virtual void ice_throw() const;
};

void throwDatabaseException(const char*, int, const Freeze::DatabaseException&);

class DatabaseConnection : public virtual IceDB::DatabaseConnection
{
public:

    DatabaseConnection(const Freeze::ConnectionPtr&);

    virtual Ice::EncodingVersion getEncoding() const;

    virtual void beginTransaction();
    virtual void commitTransaction();
    virtual void rollbackTransaction();

    Freeze::ConnectionPtr freezeConnection() const
    {
        return _connection;
    }

private:

    Freeze::ConnectionPtr _connection;
};

class ConnectionPool : public virtual IceDB::ConnectionPool
{
public:

    ConnectionPool(const Ice::CommunicatorPtr&, const std::string&);

    virtual IceDB::DatabaseConnectionPtr getConnection();
    virtual IceDB::DatabaseConnectionPtr newConnection();

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _envName;
    const IceDB::DatabaseConnectionPtr _connection;
};

template<class Dict, class Key, class Value> class Wrapper : public virtual IceDB::Wrapper<Key, Value>
{
public:

    Wrapper(const Freeze::ConnectionPtr& connection, const std::string& dbName) : 
        _dict(connection, dbName), _dbName(dbName)
    {
    }

    virtual std::map<Key, Value> getMap()
    {
        try
        {
            std::map<Key, Value> m;
            for(typename Dict::const_iterator p = _dict.begin(); p != _dict.end(); ++p)
            {
#ifdef __SUNPRO_CC             
                std::map<Key, Value>::value_type v(p->first, p->second);
                m.insert(v);
#else
                m.insert(*p);
#endif
            }
            return m;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            throwDatabaseException(__FILE__, __LINE__, ex);
            return std::map<Key, Value>(); // Keep the compiler happy.
        }
    }

    virtual void
    put(const Key& key, const Value& data)
    {
        try
        {
            _dict.put(typename Dict::value_type(key, data));
        }
        catch(const Freeze::DatabaseException& ex)
        {
            throwDatabaseException(__FILE__, __LINE__, ex);
        }

    }

    virtual Value
    find(const Key& key)
    {
        try
        {
            typename Dict::const_iterator p = _dict.find(key);
            if(p == _dict.end())
            {
                throw NotFoundException(__FILE__, __LINE__);
            }
            return p->second;
        }
        catch(const Freeze::DatabaseException& ex)
        {
            throwDatabaseException(__FILE__, __LINE__, ex);
            return Value(); // Keep the compiler happy
        }
    }

    void
    erase(const Key& key)
    {
        try
        {
            _dict.erase(key);
        }
        catch(const Freeze::DatabaseException& ex)
        {
            throwDatabaseException(__FILE__, __LINE__, ex);
        }
    }

    void
    clear()
    {
        try
        {
            _dict.clear();
        }
        catch(const Freeze::DatabaseException& ex)
        {
            throwDatabaseException(__FILE__, __LINE__, ex);
        }
    }

protected:

    Dict _dict;
    std::string _dbName;
};

}

#endif

