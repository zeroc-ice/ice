// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_OBJECTCACHE_H
#define ICE_GRID_OBJECTCACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class ObjectCache;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;

class ObjectEntry;
typedef IceUtil::Handle<ObjectEntry> ObjectEntryPtr;

class ObjectEntry : public IceUtil::Shared
{
public:
    
    ObjectEntry(Cache<Ice::Identity, ObjectEntry>&, const Ice::Identity&);

    void set(const std::string&, const ObjectInfo&);
    Ice::ObjectPrx getProxy() const;
    std::string getType() const;
    std::string getApplication() const;
    const ObjectInfo& getObjectInfo() const;

    bool canRemove();
    
private:

    std::string _application;
    ObjectInfo _info;
};
typedef IceUtil::Handle<ObjectEntry> ObjectEntryPtr;

class ObjectCache : public Cache<Ice::Identity, ObjectEntry>
{
public:

    ObjectCache(const Ice::CommunicatorPtr&);

    void add(const std::string&, const std::string&, const std::string&, const ObjectDescriptor&);
    ObjectEntryPtr get(const Ice::Identity&) const;
    ObjectEntryPtr remove(const Ice::Identity&);

    Ice::ObjectProxySeq getObjectsByType(const std::string&);
    ObjectInfoSeq getAll(const std::string&);

private:
    
    const Ice::CommunicatorPtr _communicator;
    std::map<std::string, std::set<Ice::Identity> > _types;
};

};

#endif
