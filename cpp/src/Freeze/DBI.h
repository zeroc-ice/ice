// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef FREEZE_DB_I_H
#define FREEZE_DB_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/DB.h>
#include <db.h>

#if DB_VERSION_MAJOR < 3 || (DB_VERSION_MAJOR == 3 && DB_VERSION_MINOR < 3)
#    error "Berkeley DB versions older than 3.3 are not supported!"
#endif

#ifdef _WIN32

#   if DB_VERSION_MAJOR == 3 && DB_VERSION_MINOR == 3
#       define ICE_DB_VERSION "33"
#   elif DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR == 0
#       define ICE_DB_VERSION "40"
#   elif DB_VERSION_MAJOR == 4 && DB_VERSION_MINOR == 1
#       define ICE_DB_VERSION "41"
#   else
#       error "Berkeley DB version is not supported!"
#   endif

#   ifdef _DEBUG
#       pragma comment(lib, "libdb" ICE_DB_VERSION "d.lib")
#   else
#       pragma comment(lib, "libdb" ICE_DB_VERSION ".lib")
#   endif

#endif

namespace Freeze
{

//
// Check a Berkeley DB return result, and throw an apropriate
// exception.
//
void checkBerkeleyDBReturn(int ret, const std::string&, const std::string&);

class DBEnvironmentI;
typedef IceUtil::Handle<DBEnvironmentI> DBEnvironmentIPtr;

class DBEnvironmentI : public DBEnvironment, public IceUtil::RecMutex
{
public:

    DBEnvironmentI(const ::Ice::CommunicatorPtr&, const std::string&, bool = false);
    virtual ~DBEnvironmentI();

    virtual std::string getName();
    virtual ::Ice::CommunicatorPtr getCommunicator();

    virtual DBPtr openDB(const std::string&, bool);

    virtual DBPtr openDBWithTxn(const DBTransactionPtr&, const std::string&, bool);

    virtual DBTransactionPtr startTransaction();

    virtual void close();

    virtual void sync();

private:

    // DBI needs access to add, remove & eraseDB
    friend class DBI;
    DBPtr openDBImpl(::DB_TXN*, const std::string&, bool);
    void add(const std::string&, const DBPtr&);
    void remove(const std::string&);
    void eraseDB(const std::string&);

    ::Ice::CommunicatorPtr _communicator;
    int _trace;

    ::DB_ENV* _dbEnv;

    std::string _name;
    std::string _errorPrefix;
    std::string _id;
    
    std::map<std::string, DBPtr> _dbMap;
};

class DBTransactionI : public DBTransaction, public IceUtil::Mutex
{
public:

    DBTransactionI(const ::Ice::CommunicatorPtr&, ::DB_ENV*, const std::string&);
    virtual ~DBTransactionI();

    virtual void commit();
    virtual void abort();

private:

    friend class DBEnvironmentI;
    friend class DBI;

    ::Ice::CommunicatorPtr _communicator;
    int _trace;

    ::DB_TXN* _tid;

    std::string _name;
    std::string _errorPrefix;
};


class DBI : public DB, public IceUtil::Mutex
{
public:
    
    DBI(const ::Ice::CommunicatorPtr&, const DBEnvironmentIPtr&, ::DB*, ::DB_TXN*, const std::string&, bool);
    virtual ~DBI();

    virtual std::string getName();
    virtual ::Ice::CommunicatorPtr getCommunicator();

    virtual ::Ice::Long getNumberOfRecords();

    virtual DBCursorPtr getCursor();
    virtual DBCursorPtr getCursorAtKey(const Key&);

    virtual void put(const Key&, const Value&);
    virtual bool contains(const Key&);
    virtual Value get(const Key&);
    virtual void del(const Key&);

    virtual DBCursorPtr getCursorWithTxn(const DBTransactionPtr&);
    virtual DBCursorPtr getCursorAtKeyWithTxn(const DBTransactionPtr&, const Key&);

    virtual void putWithTxn(const DBTransactionPtr&, const Key&, const Value&);
    virtual bool containsWithTxn(const DBTransactionPtr&, const Key&);
    virtual Value getWithTxn(const DBTransactionPtr&, const Key&);
    virtual void delWithTxn(const DBTransactionPtr&, const Key&);

    virtual void clear();

    virtual void close();
    virtual void remove();
    virtual void sync();

    virtual EvictionStrategyPtr createEvictionStrategy();
    virtual IdleStrategyPtr createIdleStrategy();

    virtual Freeze::EvictorPtr createEvictor(const PersistenceStrategyPtr&);

private:

    virtual DBCursorPtr getCursorImpl(::DB_TXN*);
    virtual DBCursorPtr getCursorAtKeyImpl(::DB_TXN*, const Key&);

    virtual void putImpl(::DB_TXN*, const Key&, const Value&);
    virtual bool containsImpl(::DB_TXN*, const Key&);
    virtual Value getImpl(::DB_TXN*, const Key&);
    virtual void delImpl(::DB_TXN*, const Key&);

    ::Ice::CommunicatorPtr _communicator;
    int _trace;

    DBEnvironmentIPtr _dbEnvObj;
    ::DB* _db;

    std::string _name;
    std::string _errorPrefix;
};

typedef IceUtil::Handle<DBI> DBIPtr;

class DBCursorI : public DBCursor
{
public:

    DBCursorI(const DBIPtr&, const ::Ice::CommunicatorPtr&, const std::string&, DBC*);
    ~DBCursorI();

    virtual ::Ice::CommunicatorPtr getCommunicator();

    virtual void curr(Key& key, Value& value);
    virtual void set(const Value& value);
    virtual bool next();
    virtual bool prev();
    virtual void del();
    
    virtual DBCursorPtr clone();
    virtual void close();

private:

    DBIPtr _db;
    ::Ice::CommunicatorPtr _communicator;
    int _trace;

    std::string _name;
    std::string _errorPrefix;

    DBC* _cursor;
};

}

#endif
