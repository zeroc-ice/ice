// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/BackgroundSaveEvictorI.h>
#include <Freeze/Initialize.h>
#include <IceUtil/AbstractMutex.h>
#include <IceUtil/StringUtil.h>
#include <Freeze/Util.h>

#include <Freeze/ObjectStore.h>

#include <IceUtil/Mutex.h>
#include <IceUtil/MutexPtrLock.h>

#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;

namespace
{

//
// Fatal error callback
//

Freeze::FatalErrorCallback fatalErrorCallback = 0;
IceUtil::Mutex* fatalErrorCallbackMutex = 0;

class Init
{
public:

    Init()
    {
        fatalErrorCallbackMutex = new IceUtil::Mutex;
    }

    ~Init()
    {
        delete fatalErrorCallbackMutex;
        fatalErrorCallbackMutex = 0;
    }
};
Init init;

void 
handleFatalError(const Freeze::BackgroundSaveEvictorPtr& evictor, const Ice::CommunicatorPtr& communicator)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(fatalErrorCallbackMutex);
    if(fatalErrorCallback != 0)
    {
        fatalErrorCallback(evictor, communicator);
    }
    else
    {
        ::abort();
    }
}

//
// The timer is used to ensure the streaming of some object does not take more than
// timeout ms. We only measure the time necessary to acquire the lock on the object
// (servant), not the streaming itself.
//
class WatchDogTask : public IceUtil::TimerTask
{
public:
    
    WatchDogTask(BackgroundSaveEvictorI& evictor) : _evictor(evictor)
    {
    }

    virtual void runTimerTask()
    {
        Error out(_evictor.communicator()->getLogger());
        out << "Fatal error: streaming watch dog timed out.";
        out.flush();
        handleFatalError(&_evictor, _evictor.communicator());
    }
    
private:

    BackgroundSaveEvictorI& _evictor;
};

}

//
// createEvictor functions
// 

Freeze::BackgroundSaveEvictorPtr
Freeze::createBackgroundSaveEvictor(const ObjectAdapterPtr& adapter, 
                                    const string& envName, 
                                    const string& filename,
                                    const ServantInitializerPtr& initializer,
                                    const vector<IndexPtr>& indices,
                                    bool createDb)
{
    return new BackgroundSaveEvictorI(adapter, envName, 0, filename, initializer, indices, createDb);
}

BackgroundSaveEvictorPtr
Freeze::createBackgroundSaveEvictor(const ObjectAdapterPtr& adapter, 
                                    const string& envName, 
                                    DbEnv& dbEnv, 
                                    const string& filename,
                                    const ServantInitializerPtr& initializer,
                                    const vector<IndexPtr>& indices,
                                    bool createDb)
{
    return new BackgroundSaveEvictorI(adapter, envName, &dbEnv, filename, initializer, indices, createDb);
}

FatalErrorCallback 
Freeze::registerFatalErrorCallback(FatalErrorCallback cb)
{
    IceUtilInternal::MutexPtrLock<IceUtil::Mutex> lock(fatalErrorCallbackMutex);
    FatalErrorCallback result = fatalErrorCallback;
    fatalErrorCallback = cb;
    return result;
}

//
// BackgroundSaveEvictorI
//

Freeze::BackgroundSaveEvictorI::BackgroundSaveEvictorI(const ObjectAdapterPtr& adapter, 
                                                       const string& envName, 
                                                       DbEnv* dbEnv, 
                                                       const string& filename, 
                                                       const ServantInitializerPtr& initializer,
                                                       const vector<IndexPtr>& indices,
                                                       bool createDb) :
    EvictorI<BackgroundSaveEvictorElement>(adapter, envName, dbEnv, filename, FacetTypeMap(), initializer, indices, createDb),
    IceUtil::Thread("Freeze background save evictor thread"),
    _currentEvictorSize(0),
    _savingThreadDone(false)
{
    string propertyPrefix = string("Freeze.Evictor.") + envName + '.' + _filename; 
    
    //
    // By default, we save every minute or when the size of the modified queue
    // reaches 10.
    //

    _saveSizeTrigger = _communicator->getProperties()->
        getPropertyAsIntWithDefault(propertyPrefix + ".SaveSizeTrigger", 10);

    Int savePeriod = _communicator->getProperties()->
        getPropertyAsIntWithDefault(propertyPrefix + ".SavePeriod", 60 * 1000);

    _savePeriod = IceUtil::Time::milliSeconds(savePeriod);
   
    //
    // By default, we save at most 10 * SaveSizeTrigger objects per transaction
    //
    _maxTxSize = _communicator->getProperties()->
        getPropertyAsIntWithDefault(propertyPrefix + ".MaxTxSize", 10 * _saveSizeTrigger);
    
    if(_maxTxSize <= 0)
    {
        _maxTxSize = 100;
    }   

    //
    // By default, no stream timeout
    //
    _streamTimeout = _communicator->getProperties()->
        getPropertyAsIntWithDefault(propertyPrefix+ ".StreamTimeout", 0) * 1000;
    
    if(_streamTimeout > 0)
    {
        _timer = IceInternal::getInstanceTimer(_communicator);
    }

    //
    // Start saving thread
    //
    __setNoDelete(true);
    start();
    __setNoDelete(false);
}


Ice::ObjectPrx
Freeze::BackgroundSaveEvictorI::addFacet(const ObjectPtr& servant, const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    checkServant(servant);
    DeactivateController::Guard deactivateGuard(_deactivateController);
   
    ObjectStore<BackgroundSaveEvictorElement>* store = findStore(facet, _createDb);
    
    if(store == 0)
    {
        throw NotFoundException(__FILE__, __LINE__, "addFacet: could not open database for facet '"
                                + facet + "'");
    }

    bool alreadyThere = false;

    for(;;)
    {
        //
        // Create a new entry
        //
        
        BackgroundSaveEvictorElementPtr element = new BackgroundSaveEvictorElement(*store);
        element->status = dead;
        BackgroundSaveEvictorElementPtr oldElt = store->putIfAbsent(ident, element);
      
        if(oldElt != 0)
        {
            element = oldElt;
        }

        {
            Lock sync(*this);

            if(element->stale)
            {
                //
                // Try again
                // 
                continue;
            }
            fixEvictPosition(element);

            IceUtil::Mutex::Lock lock(element->mutex);
        
            switch(element->status)
            {
                case clean:
                case created:
                case modified:
                {
                    alreadyThere = true;
                    break;
                }  
                case destroyed:
                {
                    element->status = modified;
                    element->rec.servant = servant;
                    
                    //
                    // No need to push it on the modified queue, as a destroyed object
                    // is either already on the queue or about to be saved. When saved,
                    // it becomes dead.
                    //
                    break;
                }
                case dead:
                {
                    element->status = created;
                    ObjectRecord& rec = element->rec;

                    rec.servant = servant;
                    if(store->keepStats())
                    {
                        rec.stats.creationTime = IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds();
                        rec.stats.lastSaveTime = 0;
                        rec.stats.avgSaveTime = 0;
                    }
                    addToModifiedQueue(element);
                    break;
                }
                default:
                {
                    assert(0);
                    break;
                }
            }
        }
        break; // for(;;)
    }
    
    if(alreadyThere)
    {
        AlreadyRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _communicator->identityToString(ident);
        if(!facet.empty())
        {
            ex.id += " -f " + IceUtilInternal::escapeString(facet, "");
        }
        throw ex;
    }

    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.Evictor");
        out << "added object \"" << _communicator->identityToString(ident) << "\"";
        if(!facet.empty())
        {
            out << " with facet \"" << facet << "\"";
        }
        out << " to Db \"" << _filename << "\"";
    }

    ObjectPrx obj = _adapter->createProxy(ident);
    if(!facet.empty())
    {
        obj = obj->ice_facet(facet);
    }
    return obj;
}

Ice::ObjectPtr
Freeze::BackgroundSaveEvictorI::removeFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);
   
    ObjectStore<BackgroundSaveEvictorElement>* store = findStore(facet, false);
    ObjectPtr servant = 0;

    if(store != 0)
    {
        for(;;)
        {
            //
            // Retrieve object
            //
            
            BackgroundSaveEvictorElementPtr element = store->pin(ident);
            if(element != 0)
            {
                Lock sync(*this);
                if(element->stale)
                {
                    //
                    // Try again
                    // 
                    continue;
                }
            
                fixEvictPosition(element);
                {
                    IceUtil::Mutex::Lock lock(element->mutex);
                    
                    switch(element->status)
                    {
                        case clean:
                        {
                            servant = element->rec.servant;
                            element->status = destroyed;
                            element->rec.servant = 0;
                            addToModifiedQueue(element);
                            break;
                        }
                        case created:
                        {
                            servant = element->rec.servant;
                            element->status = dead;
                            element->rec.servant = 0;
                            break;
                        }
                        case modified:
                        {
                            servant = element->rec.servant;
                            element->status = destroyed;
                            element->rec.servant = 0;
                            //
                            // Not necessary to push it on the modified queue, as a modified
                            // element is either on the queue already or about to be saved
                            // (at which point it becomes clean)
                            //
                            break;
                        }  
                        case destroyed:
                        case dead:
                        {
                            break;
                        }
                        default:
                        {
                            assert(0);
                            break;
                        }
                    }
                }
                if(element->keepCount > 0)
                {
                    assert(servant != 0);

                    element->keepCount = 0;
                    //
                    // Add to front of evictor queue
                    //
                    // Note that save evicts dead objects
                    //
                    _evictorList.push_front(element);
                    _currentEvictorSize++;
                    element->evictPosition = _evictorList.begin();
                }
            }
            break; // for(;;)  
        }
    }
    
    if(servant == 0)
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _communicator->identityToString(ident);
        if(!facet.empty())
        {
            ex.id += " -f " + IceUtilInternal::escapeString(facet, "");
        }
        throw ex;
    }

    if(_trace >= 1)
    {
        Trace out(_communicator->getLogger(), "Freeze.Evictor");
        out << "removed object \"" << _communicator->identityToString(ident) << "\"";
        if(!facet.empty())
        {
            out << " with facet \"" << facet << "\"";
        }
        out << " from Db \"" << _filename << "\"";
    }
    return servant;
}

void
Freeze::BackgroundSaveEvictorI::keep(const Identity& ident)
{
    keepFacet(ident, "");
}

void
Freeze::BackgroundSaveEvictorI::keepFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);

    bool notThere = false;

    ObjectStore<BackgroundSaveEvictorElement>* store = findStore(facet, false);
    if(store == 0)
    {
        notThere = true;
    }
    else
    {
        for(;;)
        {
            BackgroundSaveEvictorElementPtr element = store->pin(ident);
            if(element == 0)
            {
                notThere = true;
                break;
            }
            
            Lock sync(*this);
            
            if(element->stale)
            {
                //
                // try again
                //
                continue;
            }
            
            
            {
                IceUtil::Mutex::Lock lockElement(element->mutex);
                if(element->status == destroyed || element->status == dead)
                {
                    notThere = true;
                    break;
                }
            }
            
            //
            // Found!
            //

            if(element->keepCount == 0)
            {
                if(element->usageCount < 0)
                {
                    //
                    // New object
                    //
                    element->usageCount = 0;
                }
                else
                {
                    _evictorList.erase(element->evictPosition);
                    _currentEvictorSize--;
                }
                element->keepCount = 1;
            }
            else
            {
                element->keepCount++;
            }
            break;
        }
    }

    if(notThere)
    {
        NotRegisteredException ex(__FILE__, __LINE__);
        ex.kindOfObject = "servant";
        ex.id = _communicator->identityToString(ident);
        if(!facet.empty())
        {
            ex.id += " -f " + IceUtilInternal::escapeString(facet, "");
        }
        throw ex;
    }
}

void
Freeze::BackgroundSaveEvictorI::release(const Identity& ident)
{
    releaseFacet(ident, "");
}

void
Freeze::BackgroundSaveEvictorI::releaseFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);

    ObjectStore<BackgroundSaveEvictorElement>* store = findStore(facet, false);

    if(store != 0)
    {
        Lock sync(*this);
        
        BackgroundSaveEvictorElementPtr element = store->getIfPinned(ident);
        if(element != 0)
        {
            assert(!element->stale);
            if(element->keepCount > 0) 
            {
                if(--element->keepCount == 0)
                {
                    //
                    // Add to front of evictor queue
                    //
                    // Note that the element cannot be destroyed or dead since
                    // its keepCount was > 0.
                    //
                    _evictorList.push_front(element);
                    _currentEvictorSize++;
                    element->evictPosition = _evictorList.begin();
                }
                //
                // Success
                //
                return;
            }
        }
    }
    
    NotRegisteredException ex(__FILE__, __LINE__);
    ex.kindOfObject = "servant";
    ex.id = _communicator->identityToString(ident);
    if(!facet.empty())
    {
        ex.id += " -f " + IceUtilInternal::escapeString(facet, "");
    }
    throw ex;
}


bool
Freeze::BackgroundSaveEvictorI::hasFacet(const Identity& ident, const string& facet)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);
    checkIdentity(ident);
    ObjectStore<BackgroundSaveEvictorElement>* store = findStore(facet, false);

    if(store == 0)
    {
        return false;
    }

    {
        Lock sync(*this);  
        BackgroundSaveEvictorElementPtr element = store->getIfPinned(ident);
        if(element != 0)
        {
            assert(!element->stale);    
            
            IceUtil::Mutex::Lock lock(element->mutex);
            return element->status != dead && element->status != destroyed;
        }
    }
    return store->dbHasObject(ident, 0);
}

bool
Freeze::BackgroundSaveEvictorI::hasAnotherFacet(const Identity& ident, const string& facet)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);

    //
    // If the object exists in another store, throw FacetNotExistException 
    // instead of returning 0 (== ObjectNotExistException)
    // 
    StoreMap storeMapCopy;
    {
        Lock sync(*this);
        storeMapCopy = _storeMap;
    }       
        
    for(StoreMap::iterator p = storeMapCopy.begin(); p != storeMapCopy.end(); ++p)
    {
        //
        // Do not check again the given facet
        //
        if((*p).first != facet)
        { 
            ObjectStore<BackgroundSaveEvictorElement>* store = (*p).second;
            
            bool inCache = false;
            {
                Lock sync(*this);
                
                BackgroundSaveEvictorElementPtr element = store->getIfPinned(ident);
                if(element != 0)
                {
                    inCache = true;
                    assert(!element->stale);    
                    
                    IceUtil::Mutex::Lock lock(element->mutex);
                    if(element->status != dead && element->status != destroyed)
                    {
                        return true;
                    }
                }
            }
            if(!inCache)
            {
                if(store->dbHasObject(ident, 0))
                {
                    return true;
                }
            }
        }
    }
    return false;
}


ObjectPtr
Freeze::BackgroundSaveEvictorI::locateImpl(const Current& current, LocalObjectPtr& cookie)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);

    cookie = 0;

    ObjectStore<BackgroundSaveEvictorElement>* store = findStore(current.facet, false);
    if(store == 0)
    {
        if(_trace >= 2)
        {
            Trace out(_communicator->getLogger(), "Freeze.Evictor");
            out << "locate could not find a database for facet \"" << current.facet << "\"";
        }
        return 0;
    }
    
    for(;;)
    {
        BackgroundSaveEvictorElementPtr element = store->pin(current.id);
        if(element == 0)
        {
            if(_trace >= 2)
            {
                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                out << "locate could not find \"" << _communicator->identityToString(current.id) << "\" in Db \""
                    << _filename << "\"";
            }
            return 0;
        }
        
        Lock sync(*this);

        if(element->stale)
        {
            //
            // try again
            //
            continue;
        }


        IceUtil::Mutex::Lock lockElement(element->mutex);
        if(element->status == destroyed || element->status == dead)
        {
            if(_trace >= 2)
            {
                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                out << "locate found \"" << _communicator->identityToString(current.id) 
                    << "\" in the cache for database \"" << current.facet << "\" but it was dead or destroyed";
            }
            return 0;
        }

        //
        // It's a good one!
        //
        if(_trace >= 2)
        {
            Trace out(_communicator->getLogger(), "Freeze.Evictor");
            out << "locate found \"" << _communicator->identityToString(current.id) << "\" in Db \"" 
                << _filename << "\"";
        }

        fixEvictPosition(element);
        element->usageCount++;
        cookie = element;
        assert(element->rec.servant != 0);
        return element->rec.servant;
    }
}

void
Freeze::BackgroundSaveEvictorI::finished(const Current& current, const ObjectPtr& servant, const LocalObjectPtr& cookie)
{
    assert(servant != 0);
    DeactivateController::Guard deactivateGuard(_deactivateController);

    if(cookie != 0)
    {
        BackgroundSaveEvictorElementPtr element = BackgroundSaveEvictorElementPtr::dynamicCast(cookie);
        assert(element);
    
        bool enqueue = false;
        
        if((servant->ice_operationAttributes(current.operation) & 0x1) != 0)
        {
            IceUtil::Mutex::Lock lock(element->mutex);
            
            if(element->status == clean)
            {
                //
                // Assume this operation updated the object
                // 
                element->status = modified;
                enqueue = true;
            }
        }
        
        Lock sync(*this);

        //
        // Only elements with a usageCount == 0 can become stale and we own 
        // one count!
        // 
        assert(!element->stale);
        assert(element->usageCount >= 1);
        
        //
        // Decrease the usage count of the evictor queue element.
        //
        element->usageCount--;
        
        if(enqueue)
        {
            addToModifiedQueue(element);
        }
        else if(element->usageCount == 0 && element->keepCount == 0)
        {
            //
            // Evict as many elements as necessary.
            //
            evict();
        }
    }
}

void
Freeze::BackgroundSaveEvictorI::deactivate(const string&)
{
    if(_deactivateController.deactivate())
    {
        try
        {
            saveNow();

            Lock sync(*this);
     
            //
            // Set the evictor size to zero, meaning that we will evict
            // everything possible.
            //
            _evictorSize = 0;
            evict();
            
            _savingThreadDone = true;
            notifyAll();
            sync.release();
            getThreadControl().join();
            
            closeDbEnv();
        }
        catch(...)
        {
            _deactivateController.deactivationComplete();
            throw;
        }
        _deactivateController.deactivationComplete();
    }
}

Freeze::BackgroundSaveEvictorI::~BackgroundSaveEvictorI()
{
    if(!_deactivateController.deactivated())
    {
        Warning out(_communicator->getLogger());
        out << "evictor has not been deactivated";
    }
}


void
Freeze::BackgroundSaveEvictorI::run()
{
    try
    {
        for(;;)
        {
            deque<BackgroundSaveEvictorElementPtr> allObjects;
            deque<BackgroundSaveEvictorElementPtr> deadObjects;

            size_t saveNowThreadsSize = 0;
            
            {
                Lock sync(*this);

                while(!_savingThreadDone &&
                      (_saveNowThreads.size() == 0) &&
                      (_saveSizeTrigger < 0 || static_cast<Int>(_modifiedQueue.size()) < _saveSizeTrigger))
                {
                    if(_savePeriod == IceUtil::Time::milliSeconds(0))
                    {
                        wait();
                    }
                    else if(timedWait(_savePeriod) == false)
                    {
                        //
                        // Timeout, so let's save
                        //
                        break; // while
                    }                           
                }
                
                saveNowThreadsSize = _saveNowThreads.size();
                
                if(_savingThreadDone)
                {
                    assert(_modifiedQueue.size() == 0);
                    assert(saveNowThreadsSize == 0);
                    break; // for(;;)
                }
                
                //
                // Check first if there is something to do!
                //
                if(_modifiedQueue.size() == 0)
                {
                    if(saveNowThreadsSize > 0)
                    {
                        _saveNowThreads.clear();
                        notifyAll();
                    }
                    continue; // for(;;)
                }
                
                _modifiedQueue.swap(allObjects);
            }
            
            const size_t size = allObjects.size();
            
            deque<StreamedObjectPtr> streamedObjectQueue;
            
            Long streamStart = IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds();
            
            //
            // Stream each element
            //
            for(size_t i = 0; i < size; i++)
            {
                BackgroundSaveEvictorElementPtr& element = allObjects[i];
                
                bool tryAgain;
                do
                {
                    tryAgain = false;
                    ObjectPtr servant = 0;
                    
                    //
                    // These elements can't be stale as only elements with 
                    // usageCount == 0 can become stale, and the modifiedQueue
                    // (us now) owns one count.
                    //

                    IceUtil::Mutex::Lock lockElement(element->mutex);
                    Byte status = element->status;
                    
                    switch(status)
                    {
                        case created:
                        case modified:
                        {
                            servant = element->rec.servant;
                            break;
                        }   
                        case destroyed:
                        {
                            size_t index = streamedObjectQueue.size();
                            streamedObjectQueue.resize(index + 1);
                            streamedObjectQueue[index] = new StreamedObject;
                            stream(element, streamStart, streamedObjectQueue[index]);

                            element->status = dead;
                            deadObjects.push_back(element);

                            break;
                        }   
                        case dead:
                        {
                            deadObjects.push_back(element);
                            break;
                        }
                        default:
                        {
                            //
                            // Nothing to do (could be a duplicate)
                            //
                            break;
                        }
                    }
                    if(servant == 0)
                    {
                        lockElement.release();
                    }
                    else
                    {
                        IceUtil::AbstractMutex* mutex = dynamic_cast<IceUtil::AbstractMutex*>(servant.get());
                        if(mutex != 0)
                        {
                            //
                            // Lock servant and then element so that user can safely lock
                            // servant and call various Evictor operations
                            //
                            
                            IceUtil::AbstractMutex::TryLock lockServant(*mutex);
                            if(!lockServant.acquired())
                            {
                                lockElement.release();

                                IceUtil::TimerTaskPtr watchDogTask;
                                if(_timer)
                                {
                                    watchDogTask = new WatchDogTask(*this);
                                    _timer->schedule(watchDogTask, IceUtil::Time::milliSeconds(_streamTimeout));
                                }
                                lockServant.acquire();
                                if(watchDogTask)
                                {
                                    _timer->cancel(watchDogTask);
                                    watchDogTask = 0;
                                }

                                lockElement.acquire();
                                status = element->status;
                            }
  
                            switch(status)
                            {
                                case created:
                                case modified:
                                {
                                    if(servant == element->rec.servant)
                                    {
                                        size_t index = streamedObjectQueue.size();
                                        streamedObjectQueue.resize(index + 1);
                                        streamedObjectQueue[index] = new StreamedObject;
                                        stream(element, streamStart, streamedObjectQueue[index]);

                                        element->status = clean;
                                    }
                                    else
                                    {
                                        tryAgain = true;
                                    }
                                    break;
                                }
                                case destroyed:
                                {
                                    lockServant.release();
                                    
                                    size_t index = streamedObjectQueue.size();
                                    streamedObjectQueue.resize(index + 1);
                                    streamedObjectQueue[index] = new StreamedObject;
                                    stream(element, streamStart, streamedObjectQueue[index]);

                                    element->status = dead;
                                    deadObjects.push_back(element);
                                    break;
                                }   
                                case dead:
                                {
                                    deadObjects.push_back(element);
                                    break;
                                }
                                default:
                                {
                                    //
                                    // Nothing to do (could be a duplicate)
                                    //
                                    break;
                                }
                            }
                        }
                        else
                        {
                            DatabaseException ex(__FILE__, __LINE__);
			    Ice::Object& servant = *element->rec.servant;
                            ex.message = string(typeid(servant).name()) + " does not implement IceUtil::AbstractMutex";
                            throw ex;
                        }
                    }
                } while(tryAgain);
            }
            
            if(_trace >= 1)
            {
                Long now = IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds();
                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                out << "streamed " << streamedObjectQueue.size() << " objects in " 
                    << static_cast<Int>(now - streamStart) << " ms";
            }
            
            //
            // Now let's save all these streamed objects to disk using a transaction
            //
            
            //
            // Each time we get a deadlock, we reduce the number of objects to save
            // per transaction
            //
            size_t txSize = streamedObjectQueue.size();
            if(txSize > static_cast<size_t>(_maxTxSize))
            {
                txSize = static_cast<size_t>(_maxTxSize);
            }
            bool tryAgain;
            
            do
            {
                tryAgain = false;
                
                while(streamedObjectQueue.size() > 0)
                {
                    if(txSize > streamedObjectQueue.size())
                    {
                        txSize = streamedObjectQueue.size();
                    }
                    
                    Long saveStart = IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds();
                   
                    try
                    {
                        DbTxn* tx = 0;
                        _dbEnv->getEnv()->txn_begin(0, &tx, 0);

                        long txnId = 0;
                        if(_txTrace >= 1)
                        {
                            txnId = (tx->id() & 0x7FFFFFFF) + 0x80000000L;
                            Trace out(_communicator->getLogger(), "Freeze.Evictor");
                            out << "started transaction " << hex << txnId << dec << " in saving thread";
                        }

                        try
                        {       
                            for(size_t i = 0; i < txSize; i++)
                            {
                                StreamedObjectPtr obj = streamedObjectQueue[i];
                                Dbt key, value;
                                obj->key->getDbt(key);
                                if(obj->value)
                                {
                                    obj->value->getDbt(value);
                                }
                                obj->store->save(key, value, obj->status, tx);
                            }
                        }
                        catch(...)
                        {
                            tx->abort();
                            if(_txTrace >= 1)
                            {
                                Trace out(_communicator->getLogger(), "Freeze.Evictor");
                                out << "rolled back transaction " << hex << txnId << dec;
                            }
                            throw;
                        }
                        tx->commit(0);

                        if(_txTrace >= 1)
                        {
                            Trace out(_communicator->getLogger(), "Freeze.Evictor");
                            out << "committed transaction " << hex << txnId << dec;
                        }

                        streamedObjectQueue.erase(streamedObjectQueue.begin(), streamedObjectQueue.begin() + txSize);
                        
                        if(_trace >= 1)
                        {
                            Long now = IceUtil::Time::now(IceUtil::Time::Monotonic).toMilliSeconds();
                            Trace out(_communicator->getLogger(), "Freeze.Evictor");
                            out << "saved " << txSize << " objects in " 
                                << static_cast<Int>(now - saveStart) << " ms";
                        }
                    }
                    catch(const DbDeadlockException&)
                    {
                        if(_deadlockWarning)
                        {
                            Warning out(_communicator->getLogger());
                            out << "Deadlock in Freeze::BackgroundSaveEvictorI::run while writing into Db \"" + _filename
                                + "\"; retrying ...";
                        }
                        
                        tryAgain = true;
                        txSize = (txSize + 1)/2;
                    }
                    catch(const DbException& dx)
                    {
                        DatabaseException ex(__FILE__, __LINE__);
                        ex.message = dx.what();
                        throw ex;
                    }
                } 
            }
            while(tryAgain);
            
            {
                Lock sync(*this);
               
                //
                // Release usage count
                //
                for(deque<BackgroundSaveEvictorElementPtr>::iterator p = allObjects.begin();
                    p != allObjects.end(); p++)
                {
                    BackgroundSaveEvictorElementPtr& element = *p;
                    element->usageCount--;
                }
                allObjects.clear();

                for(deque<BackgroundSaveEvictorElementPtr>::iterator q = deadObjects.begin();
                    q != deadObjects.end(); q++)
                {
                    BackgroundSaveEvictorElementPtr& element = *q;
                    if(!element->stale)
                    {
                        //
                        // Can be stale when there are duplicate elements on the
                        // deadObjecst queue
                        //

                        if(!element->stale && element->usageCount == 0 && element->keepCount == 0)
                        {
                            //
                            // Get rid of unused dead elements
                            //
                            IceUtil::Mutex::Lock lockElement(element->mutex);
                            if(element->status == dead)
                            {
                                evict(element);
                            }
                        }
                    }
                }
                deadObjects.clear();
                evict();
                
                if(saveNowThreadsSize > 0)
                {
                    _saveNowThreads.erase(_saveNowThreads.begin(), _saveNowThreads.begin() + saveNowThreadsSize);
                    notifyAll();
                }
            }
        }
    }
    catch(const std::exception& ex)
    {
        Error out(_communicator->getLogger());
        out << "Saving thread killed by exception: " << ex;
        out.flush();
        handleFatalError(this, _communicator);
    }
    catch(...)
    {
        Error out(_communicator->getLogger());
        out << "Saving thread killed by unknown exception";
        out.flush();
        handleFatalError(this, _communicator);
    }
}

Freeze::TransactionIPtr
Freeze::BackgroundSaveEvictorI::beforeQuery()
{
    saveNow();
    return 0;
}

void
Freeze::BackgroundSaveEvictorI::saveNow()
{
    Lock sync(*this);
    
    IceUtil::ThreadControl myself;

    _saveNowThreads.push_back(myself);
    notifyAll();
    do
    {
        wait();
    }
    while(find(_saveNowThreads.begin(), _saveNowThreads.end(), myself) != _saveNowThreads.end());
}

void
Freeze::BackgroundSaveEvictorI::evict()
{
    //
    // Must be called with *this locked
    //

    assert(_currentEvictorSize == _evictorList.size());

    list<BackgroundSaveEvictorElementPtr>::reverse_iterator p = _evictorList.rbegin();
    
    while(_currentEvictorSize > _evictorSize)
    {
        //
        // Get the last unused element from the evictor queue.
        //
        while(p != _evictorList.rend())
        {
            if((*p)->usageCount == 0)
            {
                break; // Fine, servant is not in use (and not in the modifiedQueue)
            }
            ++p;
        }
        if(p == _evictorList.rend())
        {
            //
            // All servants are active, can't evict any further.
            //
            break;
        }

        BackgroundSaveEvictorElementPtr& element = *p;
        assert(!element->stale);
        assert(element->keepCount == 0);

        if(_trace >= 2 || (_trace >= 1 && _evictorList.size() % 50 == 0))
        {
            string facet = element->store.facet();

            Trace out(_communicator->getLogger(), "Freeze.Evictor");
            out << "evicting \"" << _communicator->identityToString(element->cachePosition->first) << "\" ";
            if(facet != "")
            {
                out << "-f \"" << facet << "\" ";
            }
            out << "from the queue\n" 
                << "number of elements in the queue: " << _currentEvictorSize;
        }
        
        //
        // Remove last unused element from the evictor queue.
        //
        element->stale = true;
        element->store.unpin(element->cachePosition);
        p = list<BackgroundSaveEvictorElementPtr>::reverse_iterator(_evictorList.erase(element->evictPosition));
        _currentEvictorSize--;
    }
}

void 
Freeze::BackgroundSaveEvictorI::fixEvictPosition(const BackgroundSaveEvictorElementPtr& element)
{
    assert(!element->stale);
    
    if(element->keepCount == 0)
    {
        if(element->usageCount < 0)
        {
            //
            // New object
            //
            element->usageCount = 0;
            _currentEvictorSize++;
        }
        else
        {
            _evictorList.erase(element->evictPosition);
        }
        _evictorList.push_front(element);
        element->evictPosition = _evictorList.begin();
    }
}

void 
Freeze::BackgroundSaveEvictorI::evict(const BackgroundSaveEvictorElementPtr& element)
{
    assert(!element->stale);
    assert(element->keepCount == 0);

    _evictorList.erase(element->evictPosition);
    _currentEvictorSize--;
    element->stale = true;
    element->store.unpin(element->cachePosition);
}


void
Freeze::BackgroundSaveEvictorI::addToModifiedQueue(const BackgroundSaveEvictorElementPtr& element)
{
    element->usageCount++;
    _modifiedQueue.push_back(element);
    
    if(_saveSizeTrigger >= 0 && static_cast<Int>(_modifiedQueue.size()) >= _saveSizeTrigger)
    {
        notifyAll();
    }
}


void
Freeze::BackgroundSaveEvictorI::stream(const BackgroundSaveEvictorElementPtr& element, Long streamStart,
                                       const StreamedObjectPtr& obj)
{
    assert(element->status != dead);
    
    obj->status = element->status;
    obj->store = &element->store;

    const Identity& ident = element->cachePosition->first;
    obj->key = new ObjectStoreBase::KeyMarshaler(ident, _communicator, _encoding);

    if(element->status != destroyed)
    {
        const bool keepStats = obj->store->keepStats();
        if(keepStats)
        {
            EvictorIBase::updateStats(element->rec.stats, streamStart);
        }
        obj->value = new ObjectStoreBase::ValueMarshaler(element->rec, _communicator, _encoding, keepStats);
    }
}

Freeze::BackgroundSaveEvictorElement::BackgroundSaveEvictorElement(ObjectRecord& r,
                                                                   ObjectStore<BackgroundSaveEvictorElement>& s) :
    store(s),
    usageCount(-1),
    keepCount(0),
    stale(true),
    rec(r),
    status(clean)
{
}

Freeze::BackgroundSaveEvictorElement::BackgroundSaveEvictorElement(ObjectStore<BackgroundSaveEvictorElement>& s) :
    store(s),
    usageCount(-1),
    keepCount(0),
    stale(true),
    status(clean)
{
    const Statistics cleanStats = { 0 };
    rec.stats = cleanStats;
}

Freeze::BackgroundSaveEvictorElement::~BackgroundSaveEvictorElement()
{
}

void 
Freeze::BackgroundSaveEvictorElement::init(ObjectStore<BackgroundSaveEvictorElement>::Position p)
{
    stale = false;
    cachePosition = p;
}
