// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_BACKGROUND_SAVE_EVICTOR_I_H
#define FREEZE_BACKGROUND_SAVE_EVICTOR_I_H

#include <Freeze/EvictorI.h>
#include <Freeze/BackgroundSaveEvictor.h>

namespace Freeze
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


class BackgroundSaveEvictorI;

//
// The WatchDogThread is used by the saving thread to ensure the
// streaming of some object does not take more than timeout ms.
// We only measure the time necessary to acquire the lock on the
// object (servant), not the streaming itself.
//

class WatchDogThread : public IceUtil::Thread, private IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    WatchDogThread(long, BackgroundSaveEvictorI&);
    
    void run();

    void activate();
    void deactivate();
    void terminate();
    
private:
    const IceUtil::Time _timeout;
    BackgroundSaveEvictorI& _evictor;
    bool _done;
    bool _active;
};

typedef IceUtil::Handle<WatchDogThread> WatchDogThreadPtr;

struct BackgroundSaveEvictorElement;
typedef IceUtil::Handle<BackgroundSaveEvictorElement> BackgroundSaveEvictorElementPtr;

struct BackgroundSaveEvictorElement : public Ice::LocalObject
{
    BackgroundSaveEvictorElement(ObjectRecord&, ObjectStore<BackgroundSaveEvictorElement>&);
    BackgroundSaveEvictorElement(ObjectStore<BackgroundSaveEvictorElement>&);
    ~BackgroundSaveEvictorElement();

    void init(ObjectStore<BackgroundSaveEvictorElement>::Position);

    //
    // Immutable
    //
    ObjectStore<BackgroundSaveEvictorElement>& store;

    //
    // Immutable once set
    //
    ObjectStore<BackgroundSaveEvictorElement>::Position cachePosition;

    //
    // Protected by EvictorI
    //
    std::list<BackgroundSaveEvictorElementPtr>::iterator evictPosition;
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


class BackgroundSaveEvictorI : public BackgroundSaveEvictor, public EvictorI<BackgroundSaveEvictorElement>, 
                               public IceUtil::Thread
{
public:

    BackgroundSaveEvictorI(const Ice::ObjectAdapterPtr&, const std::string&, DbEnv*, const std::string&, 
                           const ServantInitializerPtr&, const std::vector<IndexPtr>&, bool);
 
    virtual Ice::ObjectPrx addFacet(const Ice::ObjectPtr&, const Ice::Identity&, const std::string&);
    virtual Ice::ObjectPtr removeFacet(const Ice::Identity&, const std::string&);

    virtual void keep(const Ice::Identity&);
    virtual void keepFacet(const Ice::Identity&, const std::string&);

    virtual void release(const Ice::Identity&);
    virtual void releaseFacet(const Ice::Identity&, const std::string&);

    virtual bool hasFacet(const Ice::Identity&, const std::string&);

    virtual void finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&);
    virtual void deactivate(const std::string&);

    virtual ~BackgroundSaveEvictorI();
    

    virtual TransactionIPtr beforeQuery();

    //
    // Thread
    //
    virtual void run();

    struct StreamedObject
    {
        Key key;
        Value value;
        Ice::Byte status;
        ObjectStore<BackgroundSaveEvictorElement>* store;
    };

protected:
   
    virtual bool hasAnotherFacet(const Ice::Identity&, const std::string&);
    
    virtual Ice::ObjectPtr locateImpl(const Ice::Current&, Ice::LocalObjectPtr&);
   
    virtual void evict();

private:

    void saveNow();

    void evict(const BackgroundSaveEvictorElementPtr&);
    void addToModifiedQueue(const BackgroundSaveEvictorElementPtr&);
    void fixEvictPosition(const BackgroundSaveEvictorElementPtr&);

    void stream(const BackgroundSaveEvictorElementPtr&, Ice::Long, StreamedObject&);
  
    //
    // The _evictorList contains a list of all objects we keep,
    // with the most recently used first.
    //
    std::list<BackgroundSaveEvictorElementPtr> _evictorList;
    std::list<BackgroundSaveEvictorElementPtr>::size_type _currentEvictorSize;

    //
    // The _modifiedQueue contains a queue of all modified objects
    // Each element in the queue "owns" a usage count, to ensure the
    // element containing the pointed element remains in the cache.
    //
    std::deque<BackgroundSaveEvictorElementPtr> _modifiedQueue;

    bool _savingThreadDone;
    WatchDogThreadPtr _watchDogThread;
    
    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    std::deque<IceUtil::ThreadControl> _saveNowThreads;

    Ice::Int _saveSizeTrigger;
    Ice::Int _maxTxSize;
    IceUtil::Time _savePeriod;
};

}

#endif
