// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_OBJECTCACHE_H
#define ICE_GRID_OBJECTCACHE_H

#include <IceUtil/Mutex.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

class ObjectCache;

class ObjectEntry : public IceUtil::Shared
{
public:
    
    ObjectEntry(const ObjectInfo&, const std::string&);
    Ice::ObjectPrx getProxy() const;
    std::string getType() const;
    std::string getApplication() const;
    const ObjectInfo& getObjectInfo() const;

    bool canRemove();

private:

    const ObjectInfo _info;
    const std::string _application;
};
typedef IceUtil::Handle<ObjectEntry> ObjectEntryPtr;

class ObjectCache : public Cache<Ice::Identity, ObjectEntry>
{
public:

    ObjectCache(const Ice::CommunicatorPtr&);

    void add(const ObjectInfo&, const std::string&);
    ObjectEntryPtr get(const Ice::Identity&) const;
    void remove(const Ice::Identity&);

    Ice::ObjectProxySeq getObjectsByType(const std::string&); 
    ObjectInfoSeq getAll(const std::string&);
    ObjectInfoSeq getAllByType(const std::string&);

    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }

private:
    
    class TypeEntry
    {
    public:

        TypeEntry();

        void add(const ObjectEntryPtr&);
        bool remove(const ObjectEntryPtr&);
        
        const std::vector<ObjectEntryPtr>& getObjects() const { return _objects; }

    private:
        
        std::vector<ObjectEntryPtr> _objects;
    };

    const Ice::CommunicatorPtr _communicator;
    std::map<std::string, TypeEntry> _types;

    static std::pointer_to_unary_function<int, unsigned int> _rand;
};

};

#endif
