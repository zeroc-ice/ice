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
    
    DBI(const ::Ice::CommunicatorPtr&, const std::string&, const DBEnvIPtr&, ::DB_ENV*, ::DB*);
    virtual ~DBI();

    virtual void put(const std::string&, const ::Ice::ObjectPtr&);
    virtual ::Ice::ObjectPtr get(const std::string&);
    virtual void del(const std::string&);
    virtual void close();
    virtual EvictorPtr createEvictor();

private:

    ::Ice::CommunicatorPtr _communicator;
    std::string _name;
    DBEnvIPtr _dbenvObj;
    ::DB_ENV* _dbenv;
    ::DB* _db;
    ::Ice::LoggerPtr _logger;
    int _trace;
};

class DBEnvI : public DBEnv, public JTCRecursiveMutex
{
public:

    DBEnvI(const ::Ice::CommunicatorPtr&, const std::string&);
    virtual ~DBEnvI();

    virtual DBPtr open(const std::string&);
    virtual void close();

    void remove(const std::string&);

private:

    ::Ice::CommunicatorPtr _communicator;
    std::string _directory;
    ::DB_ENV* _dbenv;
    std::map<std::string, DBPtr> _dbmap;
    ::Ice::LoggerPtr _logger;
    int _trace;
};

}

#endif
