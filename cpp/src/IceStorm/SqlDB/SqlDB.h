// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_DATABASE_CACHE_H
#define SQL_DATABASE_CACHE_H

#include <IceStorm/DB.h>

#include <IceStorm/SqlDB/SqlLLU.h>
#include <IceStorm/SqlDB/SqlSubscriberMap.h>

#include <QtCore/QCoreApplication>

namespace IceStorm
{

class SqlConnectionPool : public SqlDB::ConnectionPool, public ConnectionPool
{
public:

    SqlConnectionPool(const Ice::CommunicatorPtr&, const std::string&, const std::string&,
                     const std::string&, int, const std::string&, const std::string&, const std::string&, 
                     const std::string&);
    virtual ~SqlConnectionPool();

    virtual LLUWrapperPtr getLLU(const IceDB::DatabaseConnectionPtr&);
    virtual SubscribersWrapperPtr getSubscribers(const IceDB::DatabaseConnectionPtr&);
    
private:

    const SqlLLUPtr _llu;
    const SqlSubscriberMapPtr _subscribers;
};
typedef IceUtil::Handle<SqlConnectionPool> SqlConnectionPoolPtr;

class SqlDBPlugin : public DatabasePlugin
{
public:

    SqlDBPlugin(const Ice::CommunicatorPtr&, int, char**);
    virtual ~SqlDBPlugin();

    virtual void initialize();
    virtual void destroy();
    
    ConnectionPoolPtr getConnectionPool(const std::string&);

private:

    const Ice::CommunicatorPtr _communicator;
};

}

#endif
