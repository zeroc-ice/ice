// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
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

class DBEnvironmentI;
typedef IceUtil::Handle<DBEnvironmentI> DBEnvironmentIPtr;

class DBEnvironmentI : public DBEnvironment, public JTCRecursiveMutex
{
public:

    DBEnvironmentI(const ::Ice::CommunicatorPtr&, const std::string&);
    virtual ~DBEnvironmentI();

    virtual std::string getName();
    virtual DBPtr openDB(const std::string&);
    virtual DBTransactionPtr startDBTransaction();
    virtual void close();

private:

    // DBI needs access to add and remove
    friend class DBI;
    void add(const std::string&, const DBPtr&);
    void remove(const std::string&);

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    int _trace;

    ::DB_ENV* _dbEnv;

    std::string _name;
    std::string _errorPrefix;

    std::map<std::string, DBPtr> _dbMap;
};

class DBTransactionI : public DBTransaction, public JTCMutex
{
public:

    DBTransactionI(const ::Ice::CommunicatorPtr&, ::DB_ENV*, const std::string&);
    virtual ~DBTransactionI();

    virtual void commit();
    virtual void abort();

private:

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    int _trace;

    ::DB_TXN* _tid;

    std::string _name;
    std::string _errorPrefix;
};

class DBI : public DB, public JTCMutex
{
public:
    
    DBI(const ::Ice::CommunicatorPtr&, const DBEnvironmentIPtr&, ::DB*, const std::string&);
    virtual ~DBI();

    virtual std::string getName();

    virtual void put(const Key&, const Value&, bool);
    virtual Value get(const Key&);
    virtual void del(const Key&);

    virtual void putServant(const std::string&, const ::Ice::ObjectPtr&, bool);
    virtual ::Ice::ObjectPtr getServant(const std::string&);
    virtual void delServant(const std::string&);

    virtual void close();

    virtual EvictorPtr createEvictor();

private:

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    int _trace;

    DBEnvironmentIPtr _dbEnvObj;
    ::DB* _db;

    std::string _name;
    std::string _errorPrefix;
};

}

#endif
