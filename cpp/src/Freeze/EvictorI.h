// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef FREEZE_EVICTOR_I_H
#define FREEZE_EVICTOR_I_H

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Freeze/Evictor.h>
#include <Freeze/IdentityObjectRecordDict.h>
#include <Freeze/SharedDbEnv.h>
#include <list>
#include <vector>
#include <deque>


namespace Freeze
{

class EvictorI : public Evictor,  public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Thread
{
public:

    EvictorI(const Ice::CommunicatorPtr, const std::string&, const std::string&, bool);
    EvictorI(const Ice::CommunicatorPtr, DbEnv&, const std::string&, bool);

    virtual ~EvictorI();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();

    virtual void saveNow();
    
    virtual void createObject(const Ice::Identity&, const Ice::ObjectPtr&);
    virtual void destroyObject(const Ice::Identity&);

    virtual void installServantInitializer(const ServantInitializerPtr&);
    virtual EvictorIteratorPtr getIterator();
    virtual bool hasObject(const Ice::Identity&);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

    //
    // Thread
    //
    virtual void run();

    //
    // Should be private, but the Sun C++ compiler forces us to
    // make them public
    //
    struct EvictorElement;
    typedef IceUtil::Handle<EvictorElement> EvictorElementPtr;
    typedef std::map<Ice::Identity, EvictorElementPtr> EvictorMap;

    struct EvictorElement : public Ice::LocalObject
    {
	//
	// WARNING: status and rec are protected by mutex
	// while position and usageCount are protected by the Evictor mutex.
	// To avoid memory-tearing issues on platforms with aggressive 
	// memory optimizations such as Alpha/Tru64, it is essential to put 
	// them in different quadwords (64 bit).
	//
	IceUtil::Mutex mutex;
	Ice::Byte status;  
	ObjectRecord rec;    // 64 bit alignment
	std::list<EvictorMap::iterator>::iterator position;
	int usageCount;
    };

private:

    void init(const std::string& envName, const std::string& dbName, bool createDb);

#if defined(_MSC_VER) && (_MSC_VER <= 1200)

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
    // New objects; can become clean, dead or destroyed
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

    void evict();
    bool dbHasObject(const Ice::Identity&);
    bool getObject(const Ice::Identity&, ObjectRecord&);
    void addToModifiedQueue(const EvictorMap::iterator&, const EvictorElementPtr&);
    void saveNowNoSync();
    
    inline void writeObjectRecordToValue(Ice::Long, ObjectRecord&, Value&);

    EvictorMap _evictorMap;

    //
    // The _evictorList contains a list of all objects we keep,
    // with the most recently used first
    //
    // Note: relies on the stability of iterators in a std::map
    //
    std::list<EvictorMap::iterator> _evictorList;
    EvictorMap::size_type _evictorSize;

    //
    // The _modifiedQueue contains a queue of all modified objects
    // Each element in the queue "owns" a usage count, to ensure the
    // pointed element remains in the map.
    //
    // Note: relies on the stability of iterators in a std::map
    //
    std::deque<EvictorMap::iterator> _modifiedQueue;

    bool _deactivated;
  
    Ice::CommunicatorPtr _communicator;

    DbEnv* _dbEnv;
    SharedDbEnvPtr _dbEnvHolder;
    std::auto_ptr<Db> _db;
    ServantInitializerPtr _initializer;
    int _trace;
    bool _noSyncAllowed;

    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    std::deque<IceUtil::ThreadControl> _saveNowThreads;

    Ice::Int _saveSizeTrigger;
    IceUtil::Time _savePeriod;
    IceUtil::Time _lastSave;
};

}

#endif
