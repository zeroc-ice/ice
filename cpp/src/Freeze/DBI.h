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

class DBEnvI;
typedef IceUtil::Handle<DBEnvI> DBEnvIPtr;

class DBI : public DB, public JTCMutex
{
public:
    
    DBI(const ::Ice::CommunicatorPtr&, const DBEnvIPtr&, ::DB*, const std::string&);
    virtual ~DBI();

    virtual std::string getName();
    virtual void put(const std::string&, const ::Ice::ObjectPtr&, bool);
    virtual ::Ice::ObjectPtr get(const std::string&);
    virtual void del(const std::string&);
    virtual void close();
    virtual EvictorPtr createEvictor();

private:

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    int _trace;

    DBEnvIPtr _dbenvObj;
    ::DB* _db;

    std::string _name;
    std::string _errorPrefix;
};

class DBEnvI : public DBEnv, public JTCRecursiveMutex
{
public:

    DBEnvI(const ::Ice::CommunicatorPtr&, const std::string&);
    virtual ~DBEnvI();

    virtual std::string getName();
    virtual DBPtr openDB(const std::string&);
    virtual TXNPtr startTXN();
    virtual void close();

private:

    // DBI needs access to add and remove
    friend class DBI;
    void add(const std::string&, const DBPtr&);
    void remove(const std::string&);

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::LoggerPtr _logger;
    int _trace;

    ::DB_ENV* _dbenv;

    std::string _name;
    std::string _errorPrefix;

    std::map<std::string, DBPtr> _dbmap;
};

class TXNI : public TXN, public JTCMutex
{
public:

    TXNI(const ::Ice::CommunicatorPtr&, ::DB_ENV*, const std::string&);
    virtual ~TXNI();

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

}

#endif
