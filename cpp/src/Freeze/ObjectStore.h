// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_OBJECT_STORE_H
#define FREEZE_OBJECT_STORE_H

#include <Ice/Ice.h>
#include <Ice/Identity.h>
#include <Freeze/DB.h>
#include <Freeze/EvictorStorage.h>
#include <Freeze/Index.h>
#include <IceUtil/Cache.h>

#include <vector>
#include <list>
#include <db_cxx.h>

namespace Freeze
{

class EvictorI;

struct EvictorElement;
typedef IceUtil::Handle<EvictorElement> EvictorElementPtr;

typedef IceUtil::Cache<Ice::Identity, EvictorElement> Cache;

class ObjectStore : public Cache
{
public:

    ObjectStore(const std::string&, bool, EvictorI*, 
		const std::vector<IndexPtr>& = std::vector<IndexPtr>(), bool = false);

    virtual ~ObjectStore();

    void close();

    bool dbHasObject(const Ice::Identity&) const;
    void save(Key& key, Value& value, Ice::Byte status, DbTxn* tx);

    static void marshal(const Ice::Identity&, Key&, const Ice::CommunicatorPtr&);
    static void unmarshal(Ice::Identity&, const Key&, const Ice::CommunicatorPtr&);
    static void marshal(const ObjectRecord&, Value&, const Ice::CommunicatorPtr&);
    static void unmarshal(ObjectRecord&, const Value&, const Ice::CommunicatorPtr&);

    //
    // For IndexI and Iterator
    //
    Db* db() const;
    const std::string& dbName() const;

    const Ice::CommunicatorPtr& communicator() const;
    EvictorI* evictor() const;
    const std::string& facet() const;

protected:

    virtual EvictorElementPtr load(const Ice::Identity&);
    virtual void pinned(const EvictorElementPtr&, Position p);

private:
    
    std::auto_ptr<Db> _db;
    std::string _facet;
    std::string _dbName;
    EvictorI* _evictor;
    std::vector<IndexPtr> _indices;
    Ice::CommunicatorPtr _communicator;
};


struct EvictorElement : public Ice::LocalObject
{

#if defined(_MSC_VER) && (_MSC_VER <= 1200) || defined(__IBMCPP__)

    enum 
    { 
      clean = 0,
      created = 1,
      modified = 2,
      destroyed = 3,
      dead = 4
    };
    
#else 
    //
    // Clean object; can become modified or destroyed
    //
    static const Ice::Byte clean = 0;

    //
    // New object; can become clean, dead or destroyed
    //
    static const Ice::Byte created = 1;

    //
    // Modified object; can become clean or destroyed
    //
    static const Ice::Byte modified = 2;

    //
    // Being saved. Can become dead or created
    //
    static const Ice::Byte destroyed = 3;

    //
    // Exists only in the Evictor; for example the object was created
    // and later destroyed (without a save in between), or it was
    // destroyed on disk but is still in use. Can become created.
    //
    static const Ice::Byte dead = 4;

#endif
    
    EvictorElement(ObjectStore&);
    ~EvictorElement();

    //
    // Immutable
    //
    ObjectStore& store;

    //
    // Immutable once set
    //
    Cache::Position cachePosition;

    //
    // Protected by EvictorI
    //
    std::list<EvictorElementPtr>::iterator evictPosition;
    int usageCount;
    int keepCount;
    bool stale;
    
    //
    // Protected by mutex
    // 
    IceUtil::Mutex mutex;
    ObjectRecord rec;
    Ice::Byte status;
};


//
// Inline member function definitions
//

inline Db* 
ObjectStore::db() const
{
    return _db.get();
}

inline const Ice::CommunicatorPtr& 
ObjectStore::communicator() const
{
    return _communicator;
}

inline EvictorI*
ObjectStore::evictor() const
{
    return _evictor;
}

inline const std::string&
ObjectStore::facet() const
{
    return _facet;
}

}

#endif

