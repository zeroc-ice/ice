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
#include <Freeze/Initialize.h>
#include <Freeze/DB.h>
#include <db.h>

namespace Freeze
{

class DBEnvI;
typedef IceUtil::Handle<DBEnvI> DBEnvIPtr;

class DBI : public DB, public JTCMutex
{
public:
    
    DBI(const ::Ice::CommunicatorPtr&, const ::Ice::PropertiesPtr&, const DBEnvIPtr&, ::DB_ENV*, ::DB*,
	const std::string&);
    virtual ~DBI();

    virtual void put(const std::string&, const ::Ice::ObjectPtr&);
    virtual ::Ice::ObjectPtr get(const std::string&);
    virtual void del(const std::string&);
    virtual void close();

private:

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::PropertiesPtr _properties;
    DBEnvIPtr _dbenvObj;
    ::DB_ENV* _dbenv;
    ::DB* _db;
    std::string _name;
};

class DBEnvI : public DBEnv, public JTCMutex
{
public:

    DBEnvI(const ::Ice::CommunicatorPtr&, const ::Ice::PropertiesPtr&);
    virtual ~DBEnvI();

    virtual DBPtr open(const std::string&);
    virtual void close();

    void remove(const std::string&);

private:

    ::Ice::CommunicatorPtr _communicator;
    ::Ice::PropertiesPtr _properties;
    ::DB_ENV* _dbenv;
    std::string _directory;
    std::map<std::string, DBPtr> _dbmap;
};

}

#endif
