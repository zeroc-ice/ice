// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_DB_I_H
#define FREEZE_DB_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/DB.h>
#include <db.h>

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

    DBEnvironmentI(const ::Ice::CommunicatorPtr&, const std::string&);
    virtual ~DBEnvironmentI();

    virtual std::string getName();
    virtual ::Ice::CommunicatorPtr getCommunicator();

    virtual DBPtr openDB(const std::string&, bool);

    virtual DBTransactionPtr startTransaction();

    virtual void close();

private:

    // DBI needs access to add, remove & eraseDB
    friend class DBI;
    void add(const std::string&, const DBPtr&);
    void remove(const std::string&);
    void eraseDB(const std::string&);

    ::Ice::CommunicatorPtr _communicator;
    int _trace;

    ::DB_ENV* _dbEnv;

    std::string _name;
    std::string _errorPrefix;
    int _id;

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

    ::Ice::CommunicatorPtr _communicator;
    int _trace;

    ::DB_TXN* _tid;

    std::string _name;
    std::string _errorPrefix;
};


class DBI : public DB, public IceUtil::Mutex
{
public:
    
    DBI(const ::Ice::CommunicatorPtr&, const DBEnvironmentIPtr&, ::DB*, const std::string&, bool);
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

    virtual void clear();

    virtual void close();
    virtual void remove();

    virtual EvictorPtr createEvictor(EvictorPersistenceMode);

private:

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
