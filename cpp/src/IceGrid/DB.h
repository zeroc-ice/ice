// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DATABASE_CACHE_H
#define DATABASE_CACHE_H

#include <Ice/Plugin.h>
#include <IceDB/IceDB.h>
#include <IceGrid/Admin.h>

namespace IceGrid
{

class ApplicationsWrapper : public virtual IceDB::Wrapper<std::string, ApplicationInfo>
{
};
typedef IceUtil::Handle<ApplicationsWrapper> ApplicationsWrapperPtr;

class AdaptersWrapper : public virtual IceDB::Wrapper<std::string, AdapterInfo>
{
public:

    virtual std::vector<AdapterInfo> findByReplicaGroupId(const std::string&) = 0;
};
typedef IceUtil::Handle<AdaptersWrapper> AdaptersWrapperPtr;

class ObjectsWrapper : public virtual IceDB::Wrapper<Ice::Identity, ObjectInfo>
{
public:

    virtual std::vector<ObjectInfo> findByType(const std::string&) = 0;
};
typedef IceUtil::Handle<ObjectsWrapper> ObjectsWrapperPtr;

class DatabaseCache : virtual public IceDB::DatabaseCache
{
public:

    virtual ApplicationsWrapperPtr getApplications(const IceDB::DatabaseConnectionPtr&) = 0;
    virtual AdaptersWrapperPtr getAdapters(const IceDB::DatabaseConnectionPtr&) = 0;
    virtual ObjectsWrapperPtr getObjects(const IceDB::DatabaseConnectionPtr&) = 0;
    virtual ObjectsWrapperPtr getInternalObjects(const IceDB::DatabaseConnectionPtr&) = 0;
};
typedef IceUtil::Handle<DatabaseCache> DatabaseCachePtr;

class DatabasePlugin : virtual public Ice::Plugin
{
public:

    virtual DatabaseCachePtr getDatabaseCache() = 0;
};
typedef IceUtil::Handle<DatabasePlugin> DatabasePluginPtr;

}


#endif
