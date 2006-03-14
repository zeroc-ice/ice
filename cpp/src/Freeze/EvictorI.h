// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

class EvictorI;

//
// Helper class to prevent deactivation while the Evictor is in use,
// and to queue deactivate() calls.
//
class DeactivateController : private IceUtil::Monitor<IceUtil::Mutex>
{
public:

    //
    // Prevents deactivation; the constructor raises 
    // EvictorDeactivatedException if _deactivated or _deactivating is true.
    //
    class Guard
    {
    public:
	Guard(DeactivateController&);
	~Guard();

    private:
	DeactivateController& _controller;
    };

    DeactivateController(EvictorI*);    
    
    //
    // Used mostly in asserts
    //
    bool deactivated() const;

    //
    // Returns true if this thread is supposed to do the deactivation and
    // call deactivationComplete() once done.
    //
    bool deactivate();

    void deactivationComplete();

private:
    
    friend class Guard;

    EvictorI* _evictor;
    bool _deactivating;
    bool _deactivated;
    int _guardCount;
};


class EvictorI;

//
// The WatchDogThread is used by the saving thread to ensure the
// streaming of some object does not take more than timeout ms.
// We only measure the time necessary to acquire the lock on the
// object (servant), not the streaming itself.
//

class WatchDogThread : public IceUtil::Thread, private IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    WatchDogThread(long, EvictorI&);
    
    void run();

    void activate();
    void deactivate();
    void terminate();
    
private:
    const IceUtil::Time _timeout;
    EvictorI& _evictor;
    bool _done;
    bool _active;
};

typedef IceUtil::Handle<WatchDogThread> WatchDogThreadPtr;


class EvictorI : public Evictor,  public IceUtil::Monitor<IceUtil::Mutex>, public IceUtil::Thread
{
public:

    EvictorI(const Ice::ObjectAdapterPtr&, const std::string&, DbEnv*, const std::string&, 
	     const ServantInitializerPtr&, const std::vector<IndexPtr>&, bool);

    virtual ~EvictorI();

    virtual void setSize(Ice::Int);
    virtual Ice::Int getSize();
   
    virtual Ice::ObjectPrx add(const Ice::ObjectPtr&, const Ice::Identity&);
    virtual Ice::ObjectPrx addFacet(const Ice::ObjectPtr&, const Ice::Identity&, const std::string&);
    virtual void createObject(const Ice::Identity&, const Ice::ObjectPtr&);

    virtual Ice::ObjectPtr remove(const Ice::Identity&);
    virtual Ice::ObjectPtr removeFacet(const Ice::Identity&, const std::string&);
    virtual void destroyObject(const Ice::Identity&);

    virtual void keep(const Ice::Identity&);
    virtual void keepFacet(const Ice::Identity&, const std::string&);
    virtual void release(const Ice::Identity&);
    virtual void releaseFacet(const Ice::Identity&, const std::string&);

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

    DeactivateController& deactivateController();
    const Ice::CommunicatorPtr& communicator() const;
    const SharedDbEnvPtr& dbEnv() const;
    const std::string& filename() const;

    bool deadlockWarning() const;
    Ice::Int trace() const;


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

    Ice::ObjectPtr locateImpl(const Ice::Current&, Ice::LocalObjectPtr&);
    bool hasFacetImpl(const Ice::Identity&, const std::string&);
    bool hasAnotherFacet(const Ice::Identity&, const std::string&);

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

    DeactivateController _deactivateController;
    bool _savingThreadDone;
    WatchDogThreadPtr _watchDogThread;

    Ice::ObjectAdapterPtr _adapter;
    Ice::CommunicatorPtr _communicator;

    ServantInitializerPtr _initializer;
    
    SharedDbEnvPtr _dbEnv;

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

    Ice::ObjectPtr _pingObject;
};


inline DeactivateController&
EvictorI::deactivateController()
{
    return _deactivateController;
}

inline const Ice::CommunicatorPtr&
EvictorI::communicator() const
{
    return _communicator;
}

inline const SharedDbEnvPtr&
EvictorI::dbEnv() const
{
    return _dbEnv;
}

inline bool
EvictorI::deadlockWarning() const
{
    return _deadlockWarning;
}

inline Ice::Int
EvictorI::trace() const
{
    return _trace;
}

}

#endif
