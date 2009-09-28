// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DATABASE_CACHE_H
#define DATABASE_CACHE_H

#ifdef QTSQL
#  include <IceSQL/SqlTypes.h>
#  include <IceGrid/SqlStringApplicationInfoDict.h>
#  include <IceGrid/SqlStringAdapterInfoDict.h>
#  include <IceGrid/SqlIdentityObjectInfoDict.h>
#else
#  include <Freeze/Freeze.h>
#endif

namespace IceGrid
{

#ifdef QTSQL

class DatabaseCache : public IceSQL::DatabaseCache
{
public:

    DatabaseCache(const Ice::CommunicatorPtr&, const std::string&, const std::string&, const std::string&);

    const SqlStringApplicationInfoDictPtr applications;
    const SqlStringAdapterInfoDictPtr adapters;
    const SqlIdentityObjectInfoDictPtr objects;
    const SqlIdentityObjectInfoDictPtr internalObjects;
};

typedef IceUtil::Handle<DatabaseCache> DatabaseCachePtr;

#else

typedef Freeze::ConnectionPtr DatabaseConnectionPtr;

class DatabaseCache : public IceUtil::Shared
{
public:

    DatabaseCache(const Ice::CommunicatorPtr&, const std::string&, const std::string&, const std::string&);

    DatabaseConnectionPtr getConnection();
    DatabaseConnectionPtr newConnection();

private:

    const Ice::CommunicatorPtr _communicator;
    const std::string _envName;
    const DatabaseConnectionPtr _connection;
};

typedef IceUtil::Handle<DatabaseCache> DatabaseCachePtr;

#endif


}

#endif
