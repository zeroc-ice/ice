// **********************************************************************
//
// Copyright (c) 2003-2004
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
#include <Freeze/Freeze.h>
#include <Freeze/ObjectStore.h>
#include <Freeze/SharedDbEnv.h>
#include <Freeze/Index.h>
#include <Freeze/DB.h>
#include <list>
#include <vector>
#include <deque>


namespace Freeze
{

class EvictorI : public Evictor,  public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Thread
{
public:

    EvictorI(const Ice::ObjectAdapterPtr&, const std::string&, const std::string&, 
	     const ServantInitializerPtr&, const std::vector<IndexPtr>&, bool);

    EvictorI(const Ice::ObjectAdapterPtr&, const std::string&, DbEnv&, const std::string&, 
	     const ServantInitializerPtr&, const std::vector<IndexPtr>&, bool);

    virtual ~EvictorI();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();
   
    virtual Ice::ObjectPrx add(const Ice::ObjectPtr&, const Ice::Identity&);
    virtual Ice::ObjectPrx addFacet(const Ice::ObjectPtr&, const Ice::Identity&, const std::string&);

    virtual void remove(const Ice::Identity&);
    virtual void removeFacet(const Ice::Identity&, const std::string&);
    
    virtual bool hasObject(const Ice::Identity&);
    virtual bool hasFacet(const Ice::Identity&, const std::string&);

    virtual EvictorIteratorPtr getIterator(const std::string&, Ice::Int);

    virtual Ice::ObjectPtr locate(const Ice::Current&, Ice::LocalObjectPtr&);
    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

    //
    // Thread
    //
    virtual void run();

    //
    // Accessors for other classes
    //
    void saveNow();

    const Ice::CommunicatorPtr& communicator() const;
    DbEnv* dbEnv() const;
    const std::string& filename() const;

    bool deadlockWarning() const;

    void initialize(const Ice::Identity&, const std::string&, const Ice::ObjectPtr&);

    
    struct StreamedObject
    {
	Key key;
	Value value;
	Ice::Byte status;
	ObjectStore* store;
    };

    
    static std::string defaultDb; 
    static std::string indexPrefix; 

private:

    void init(const std::string& envName, const std::vector<IndexPtr>&);

    Ice::ObjectPtr locateImpl(const Ice::Current&, Ice::LocalObjectPtr&);

    void evict();
    void evict(const EvictorElementPtr&);
    void addToModifiedQueue(const EvictorElementPtr&);
    void fixEvictPosition(const EvictorElementPtr&);

    void stream(const EvictorElementPtr&, Ice::Long, StreamedObject&);
    void saveNowNoSync();

    ObjectStore* findStore(const std::string&) const;

    std::vector<std::string> allDbs() const;

    
    typedef std::map<std::string, ObjectStore*> StoreMap;
    StoreMap _storeMap;

    //
    // The _evictorList contains a list of all objects we keep,
    // with the most recently used first.
    //
    std::list<EvictorElementPtr> _evictorList;
    std::list<EvictorElementPtr>::size_type _evictorSize;
    std::list<EvictorElementPtr>::size_type _currentEvictorSize;

    //
    // The _modifiedQueue contains a queue of all modified objects
    // Each element in the queue "owns" a usage count, to ensure the
    // element containing the pointed element remains in the cache.
    //
    std::deque<EvictorElementPtr> _modifiedQueue;

    bool _deactivated;

    Ice::ObjectAdapterPtr _adapter;
    Ice::CommunicatorPtr _communicator;

    ServantInitializerPtr _initializer;
    
    DbEnv* _dbEnv;
    SharedDbEnvPtr _dbEnvHolder;

    std::string _filename;
    bool _createDb;

    Ice::Int _trace;

    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    std::deque<IceUtil::ThreadControl> _saveNowThreads;

    Ice::Int _saveSizeTrigger;
    Ice::Int _maxTxSize;
    IceUtil::Time _savePeriod;

    bool _deadlockWarning;
};

inline const Ice::CommunicatorPtr&
EvictorI::communicator() const
{
    return _communicator;
}

inline DbEnv*
EvictorI::dbEnv() const
{
    return _dbEnv;
}

inline bool
EvictorI::deadlockWarning() const
{
    return _deadlockWarning;
}

}

#endif
