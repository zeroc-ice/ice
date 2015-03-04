// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef SQL_DATABASE_CACHE_H
#define SQL_DATABASE_CACHE_H

#include <IceGrid/DB.h>

#include <IceGrid/SqlDB/SqlStringApplicationInfoDict.h>
#include <IceGrid/SqlDB/SqlStringAdapterInfoDict.h>
#include <IceGrid/SqlDB/SqlIdentityObjectInfoDict.h>

#include <QtCore/QCoreApplication>

namespace IceGrid
{

class SqlConnectionPool : public SqlDB::ConnectionPool, public ConnectionPool
{
public:

    SqlConnectionPool(const Ice::CommunicatorPtr&, const std::string&, const std::string&,
                      const std::string&, int, const std::string&, const std::string&, const std::string&,
                      const std::string&);
    virtual ~SqlConnectionPool();

    virtual ApplicationsWrapperPtr getApplications(const IceDB::DatabaseConnectionPtr&);
    virtual AdaptersWrapperPtr getAdapters(const IceDB::DatabaseConnectionPtr&);
    virtual ObjectsWrapperPtr getObjects(const IceDB::DatabaseConnectionPtr&);
    virtual ObjectsWrapperPtr getInternalObjects(const IceDB::DatabaseConnectionPtr&);
    
    virtual std::map<std::string, Ice::Long> getSerials() const
    {
        return std::map<std::string, Ice::Long>(); // Serials not supported.
    }

private:

    const SqlStringApplicationInfoDictPtr _applications;
    const SqlStringAdapterInfoDictPtr _adapters;
    const SqlIdentityObjectInfoDictPtr _objects;
    const SqlIdentityObjectInfoDictPtr _internalObjects;
};
typedef IceUtil::Handle<SqlConnectionPool> SqlConnectionPoolPtr;

class SqlDBPlugin : public DatabasePlugin
{
public:

    SqlDBPlugin(const Ice::CommunicatorPtr&, int, char**);
    virtual ~SqlDBPlugin();

    virtual void initialize();
    virtual void destroy();
    
    bool initDB();
    ConnectionPoolPtr getConnectionPool();

private:

    const Ice::CommunicatorPtr _communicator;
    SqlConnectionPoolPtr _connectionPool;
    QCoreApplication* _qtApp;
};

}

#endif
