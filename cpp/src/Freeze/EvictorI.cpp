// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Freeze/EvictorI.h>
#include <Freeze/Initialize.h>
#include <IceUtil/AbstractMutex.h>
#include <IceUtil/StringUtil.h>
#include <Freeze/Util.h>
#include <Freeze/EvictorIteratorI.h>
#include <Freeze/PingObject.h>

#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;


//
// Static members
//

string Freeze::EvictorI::defaultDb = "$default";
string Freeze::EvictorI::indexPrefix = "$index:";


//
// createEvictor functions
// 

Freeze::EvictorPtr
Freeze::createEvictor(const ObjectAdapterPtr& adapter, 
		      const string& envName, 
		      const string& filename,
		      const ServantInitializerPtr& initializer,
		      const vector<IndexPtr>& indices,
		      bool createDb)
{
    return new EvictorI(adapter, envName, 0, filename, initializer, indices, createDb);
}

Freeze::EvictorPtr
Freeze::createEvictor(const ObjectAdapterPtr& adapter, 
		      const string& envName, 
		      DbEnv& dbEnv, 
		      const string& filename,
		      const ServantInitializerPtr& initializer,
		      const vector<IndexPtr>& indices,
		      bool createDb)
{
    return new EvictorI(adapter, envName, &dbEnv, filename, initializer, indices, createDb);
}

//
// Fatal error callback
//

static Freeze::FatalErrorCallback fatalErrorCallback = 0;
static IceUtil::StaticMutex fatalErrorCallbackMutex = ICE_STATIC_MUTEX_INITIALIZER;

Freeze::FatalErrorCallback 
Freeze::registerFatalErrorCallback(Freeze::FatalErrorCallback cb)
{
    IceUtil::StaticMutex::Lock lock(fatalErrorCallbackMutex);
    FatalErrorCallback result = fatalErrorCallback;
    return result;
}

static void 
handleFatalError(const Freeze::EvictorPtr& evictor, const Ice::CommunicatorPtr& communicator)
{
    IceUtil::StaticMutex::Lock lock(fatalErrorCallbackMutex);
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
// Helper functions
//

inline void
checkIdentity(const Identity& ident)
{
    if(ident.name.size() == 0)
    {
        IllegalIdentityException e(__FILE__, __LINE__);
        e.id = ident;
        throw e;
    }
}

//
// DeactivateController
//

Freeze::DeactivateController::Guard::Guard(DeactivateController& controller) :
    _controller(controller)
{
    Lock sync(controller);
    if(controller._deactivated || _controller._deactivating)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }
    controller._guardCount++;
}

Freeze::DeactivateController::Guard::~Guard()
{
    Lock sync(_controller);
    _controller._guardCount--;
    if(_controller._deactivating && _controller._guardCount == 0)
    {
	//
	// Notify all the threads -- although we only want to
	// reach the thread doing the deactivation.
	//
	_controller.notifyAll();
    }
}

Freeze::DeactivateController::DeactivateController(EvictorI* evictor) :
    _evictor(evictor),
    _deactivating(false),
    _deactivated(false),
    _guardCount(0)
{
}

bool
Freeze::DeactivateController::deactivated() const
{
    Lock sync(*this);
    return _deactivated || _deactivating;
}

bool
Freeze::DeactivateController::deactivate()
{
    Lock sync(*this);

    if(_deactivated)
    {
	return false;
    }
    
    if(_deactivating)
    {
	//
	// Wait for deactivated
	//
	while(!_deactivated)
	{
	    wait();
	}
	return false;
    }
    else
    {
	_deactivating = true;
	while(_guardCount > 0)
	{
	    if(_evictor->trace() >= 1)
	    {
		Trace out(_evictor->communicator()->getLogger(), "Freeze.Evictor");
		out << "*** Waiting for " << _guardCount << " threads to complete before starting deactivation.";
	    }

	    wait();
	}

	if(_evictor->trace() >= 1)
	{
	    Trace out(_evictor->communicator()->getLogger(), "Freeze.Evictor");
	    out << "Starting deactivation.";
	}
	return true;
    }
}

void
Freeze::DeactivateController::deactivationComplete()
{
    if(_evictor->trace() >= 1)
    {
	Trace out(_evictor->communicator()->getLogger(), "Freeze.Evictor");
	out << "Deactivation complete.";
    }

    Lock sync(*this);
    _deactivated = true;
    _deactivating = false;
    notifyAll();
}

//
// WatchDogThread
//

Freeze::WatchDogThread::WatchDogThread(long timeout, EvictorI& evictor) :
    _timeout(IceUtil::Time::milliSeconds(timeout)),
    _evictor(evictor),
    _done(false),
    _active(false)     
{
}
    

void 
Freeze::WatchDogThread::run()
{
    Lock sync(*this);

    while(!_done)
    {
	if(_active)
	{
	    if(timedWait(_timeout) == false && _active && !_done)
	    {
		Error out(_evictor.communicator()->getLogger());
		out << "Fatal error: streaming watch dog thread timed out.";
		out.flush();
		handleFatalError(&_evictor, _evictor.communicator());
	    }
	}
	else
	{
	    wait();
	}
    }
}

void Freeze::WatchDogThread::activate()
{
    Lock sync(*this);
    _active = true;
    notify();
}

void Freeze::WatchDogThread::deactivate()
{
    Lock sync(*this);
    _active = false;
    notify();
}
 
void 
Freeze::WatchDogThread::terminate()
{
    Lock sync(*this);
    _done = true;
    notify();
}


//
// EvictorI
//

Freeze::EvictorI::EvictorI(const ObjectAdapterPtr& adapter, 
			   const string& envName, 
			   DbEnv* dbEnv, 
			   const string& filename, 
			   const ServantInitializerPtr& initializer,
			   const vector<IndexPtr>& indices,
			   bool createDb) :
    
    _evictorSize(10),
    _currentEvictorSize(0),
    _deactivateController(this),
    _savingThreadDone(false),
    _adapter(adapter),
    _communicator(adapter->getCommunicator()),
    _initializer(initializer),
    _dbEnv(SharedDbEnv::get(_communicator, envName, dbEnv)),
    _filename(filename),
    _createDb(createDb),
    _trace(0),
    _pingObject(new PingObject)
{
    _trace = _communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Evictor");
    _deadlockWarning = (_communicator->getProperties()->getPropertyAsInt("Freeze.Warn.Deadlocks") != 0);
   
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

    bool populateEmptyIndices = 
	(_communicator->getProperties()->
	 getPropertyAsIntWithDefault(propertyPrefix + ".PopulateEmptyIndices", 0) != 0);
	
    //
    // Instantiate all Dbs in 2 steps:
    // (1) iterate over the indices and create ObjectStore with indices
    // (2) open ObjectStores without indices
    //

    vector<string> dbs = allDbs();

    //
    // Add default db in case it's not there
    //
    dbs.push_back(defaultDb);

    
    for(vector<IndexPtr>::const_iterator i = indices.begin(); i != indices.end(); ++i)
    {
	string facet = (*i)->facet();

	StoreMap::iterator q = _storeMap.find(facet);
	if(q == _storeMap.end())
	{
	    //
	    // New db
	    //

	    vector<IndexPtr> storeIndices;
 
	    for(vector<IndexPtr>::const_iterator r = i; r != indices.end(); ++r)
	    {
		if((*r)->facet() == facet)
		{
		    storeIndices.push_back(*r);
		}
	    }
	    ObjectStore* store = new ObjectStore(facet, _createDb, this, storeIndices, populateEmptyIndices);
	    _storeMap.insert(StoreMap::value_type(facet, store));
	}
    }
    
    
    for(vector<string>::iterator p = dbs.begin(); p != dbs.end(); ++p)
    {
	string facet = *p;
	if(facet == defaultDb)
	{
	    facet = "";
	}
	
	pair<StoreMap::iterator, bool> ir = 
	    _storeMap.insert(StoreMap::value_type(facet, 0));

	if(ir.second)
	{
	    ir.first->second = new ObjectStore(facet, _createDb, this);
	}
    }

    //
    // By default, no stream timeout
    //
    long streamTimeout = _communicator->getProperties()->
	getPropertyAsIntWithDefault(propertyPrefix+ ".StreamTimeout", 0) * 1000;
    
    if(streamTimeout > 0)
    {
	_watchDogThread = new WatchDogThread(streamTimeout, *this);
	_watchDogThread->start();
    }

    //
    // Start saving thread
    //
    start();
}

Freeze::EvictorI::~EvictorI()
{
    if(!_deactivateController.deactivated())
    {
	Warning out(_communicator->getLogger());
	out << "evictor has not been deactivated";

	//
	// Need to deactivate to save objects and join saving thread
	//
	deactivate("");
    }

    //
    // Delete all the ObjectStore*
    //
    for(StoreMap::iterator p = _storeMap.begin(); p != _storeMap.end(); ++p)
    {
	delete (*p).second;
    }
}

void
Freeze::EvictorI::setSize(Int evictorSize)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);

    Lock sync(*this);

    //
    // Ignore requests to set the evictor size to values smaller than zero.
    //
    if(evictorSize < 0)
    {
	return;
    }

    //
    // Update the evictor size.
    //
    _evictorSize = static_cast<size_t>(evictorSize);

    //
    // Evict as many elements as necessary.
    //
    evict();
}

Int
Freeze::EvictorI::getSize()
{
    Lock sync(*this);
    return static_cast<Int>(_evictorSize);
}


Ice::ObjectPrx
Freeze::EvictorI::add(const ObjectPtr& servant, const Identity& ident)
{
    return addFacet(servant, ident, "");
}

Ice::ObjectPrx
Freeze::EvictorI::addFacet(const ObjectPtr& servant, const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);
   
    ObjectStore* store = 0;
    
    for(;;)
    {
	{
	    Lock sync(*this);
	     
	    StoreMap::iterator p = _storeMap.find(facet);
	    if(p == _storeMap.end())
	    {
		if(store != 0)
		{
		    _storeMap.insert(StoreMap::value_type(facet, store));
		}
	    }
	    else
	    {
		delete store;
		store = (*p).second;
		assert(store != 0);
	    }
	}
	
	if(store == 0)
	{
	    assert(facet != "");
	    store = new ObjectStore(facet, _createDb, this);
	    // loop
	}
	else
	{
	    break; // for(;;)
	}
    }

    assert(store != 0);
    bool alreadyThere = false;

    for(;;)
    {
	//
	// Create a new entry
	//
	
	EvictorElementPtr element = new EvictorElement(*store);
	element->status = EvictorElement::dead;
	EvictorElementPtr oldElt = store->putIfAbsent(ident, element);
      
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
		case EvictorElement::clean:
		case EvictorElement::created:
		case EvictorElement::modified:
		{
		    alreadyThere = true;
		    break;
		}  
		case EvictorElement::destroyed:
		{
		    element->status = EvictorElement::modified;
		    element->rec.servant = servant;
		    
		    //
		    // No need to push it on the modified queue, as a destroyed object
		    // is either already on the queue or about to be saved. When saved,
		    // it becomes dead.
		    //
		    break;
		}
		case EvictorElement::dead:
		{
		    element->status = EvictorElement::created;
		    ObjectRecord& rec = element->rec;

		    rec.servant = servant;
		    rec.stats.creationTime = IceUtil::Time::now().toMilliSeconds();
		    rec.stats.lastSaveTime = 0;
		    rec.stats.avgSaveTime = 0;

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
	ex.id = identityToString(ident);
	if(!facet.empty())
	{
	    ex.id += " -f " + IceUtil::escapeString(facet, "");
	}
	throw ex;
    }

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "added object \"" << ident << "\"";
	if(!facet.empty())
	{
	    out << " with facet \"" << facet << "\"";
	}
    }

    ObjectPrx obj = _adapter->createProxy(ident);
    if(!facet.empty())
    {
	obj = obj->ice_newFacet(facet);
    }
    return obj;
}

//
// Deprecated
//
void
Freeze::EvictorI::createObject(const Identity& ident, const ObjectPtr& servant)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);
  
    ObjectStore* store = findStore("");
    assert(store != 0);
  
    for(;;)
    {
	//
	// Create a new entry
	//
	
	EvictorElementPtr element = new EvictorElement(*store);
	element->status = EvictorElement::dead;
	EvictorElementPtr oldElt = store->putIfAbsent(ident, element);
      
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
		case EvictorElement::clean:
		{
		    element->status = EvictorElement::modified;
		    element->rec.servant = servant;
		    addToModifiedQueue(element);
		    break;
		}
		case EvictorElement::created:
		case EvictorElement::modified:
		{
		    element->rec.servant = servant;
		    break;
		}  
		case EvictorElement::destroyed:
		{
		    element->status = EvictorElement::modified;
		    element->rec.servant = servant;
		    
		    //
		    // No need to push it on the modified queue, as a destroyed object
		    // is either already on the queue or about to be saved. When saved,
		    // it becomes dead.
		    //
		    break;
		}
		case EvictorElement::dead:
		{
		    element->status = EvictorElement::created;
		    ObjectRecord& rec = element->rec;

		    rec.servant = servant;
		    rec.stats.creationTime = IceUtil::Time::now().toMilliSeconds();
		    rec.stats.lastSaveTime = 0;
		    rec.stats.avgSaveTime = 0;

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

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "added object \"" << ident << "\"";
    }
}

Ice::ObjectPtr
Freeze::EvictorI::remove(const Identity& ident)
{
    return removeFacet(ident, "");
}

Ice::ObjectPtr
Freeze::EvictorI::removeFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);
   
    ObjectStore* store = findStore(facet);
    ObjectPtr servant = 0;

    if(store != 0)
    {
	for(;;)
	{
	    //
	    // Retrieve object
	    //
	    
	    EvictorElementPtr element = store->pin(ident);
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
			case EvictorElement::clean:
			{
			    servant = element->rec.servant;
			    element->status = EvictorElement::destroyed;
			    element->rec.servant = 0;
			    addToModifiedQueue(element);
			    break;
			}
			case EvictorElement::created:
			{
			    servant = element->rec.servant;
			    element->status = EvictorElement::dead;
			    element->rec.servant = 0;
			    break;
			}
			case EvictorElement::modified:
			{
			    servant = element->rec.servant;
			    element->status = EvictorElement::destroyed;
			    element->rec.servant = 0;
			    //
			    // Not necessary to push it on the modified queue, as a modified
			    // element is either on the queue already or about to be saved
			    // (at which point it becomes clean)
			    //
			    break;
			}  
			case EvictorElement::destroyed:
			case EvictorElement::dead:
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
	ex.id = identityToString(ident);
	if(!facet.empty())
	{
	    ex.id += " -f " + IceUtil::escapeString(facet, "");
	}
	throw ex;
    }

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "removed object \"" << ident << "\"";
	if(!facet.empty())
	{
	    out << " with facet \"" << facet << "\"";
	}
    }
    return servant;
}

//
// Deprecated
//
void
Freeze::EvictorI::destroyObject(const Identity& ident)
{
    checkIdentity(ident);

    try
    {
	remove(ident);
    }
    catch(NotRegisteredException&)
    {
	//
	// Ignored
	//
    }
}

void
Freeze::EvictorI::keep(const Identity& ident)
{
    keepFacet(ident, "");
}

void
Freeze::EvictorI::keepFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);

    bool notThere = false;

    ObjectStore* store = findStore(facet);
    if(store == 0)
    {
	notThere = true;
    }
    else
    {
	for(;;)
	{
	    EvictorElementPtr element = store->pin(ident);
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
		if(element->status == EvictorElement::destroyed || element->status == EvictorElement::dead)
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
	ex.id = identityToString(ident);
	if(!facet.empty())
	{
	    ex.id += " -f " + IceUtil::escapeString(facet, "");
	}
	throw ex;
    }
}

void
Freeze::EvictorI::release(const Identity& ident)
{
    releaseFacet(ident, "");
}

void
Freeze::EvictorI::releaseFacet(const Identity& ident, const string& facet)
{
    checkIdentity(ident);
    DeactivateController::Guard deactivateGuard(_deactivateController);

    {
	Lock sync(*this);
	
	StoreMap::iterator p = _storeMap.find(facet);
	if(p != _storeMap.end())
	{
	    ObjectStore* store = (*p).second;
	    
	    EvictorElementPtr element = store->getIfPinned(ident);
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
    }
    
    NotRegisteredException ex(__FILE__, __LINE__);
    ex.kindOfObject = "servant";
    ex.id = identityToString(ident);
    if(!facet.empty())
    {
	ex.id += " -f " + IceUtil::escapeString(facet, "");
    }
    throw ex;
}

EvictorIteratorPtr
Freeze::EvictorI::getIterator(const string& facet, Int batchSize)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);

    ObjectStore* store = 0;
    {
	Lock sync(*this);
	
	StoreMap::iterator p = _storeMap.find(facet);
	if(p != _storeMap.end())
	{
	    store = (*p).second;
	    saveNowNoSync();
	}
    }
    return new EvictorIteratorI(store, batchSize);
}

bool
Freeze::EvictorI::hasObject(const Identity& ident)
{
    return hasFacet(ident, "");
}

bool
Freeze::EvictorI::hasFacet(const Identity& ident, const string& facet)
{
    DeactivateController::Guard deactivateGuard(_deactivateController);
    return hasFacetImpl(ident, facet);
}

bool
Freeze::EvictorI::hasFacetImpl(const Identity& ident, const string& facet)
{
    //
    // Must be called with _deactivateController locked.
    //

    checkIdentity(ident);
    ObjectStore* store = 0;

    {
	Lock sync(*this);
       
	StoreMap::iterator p = _storeMap.find(facet);
	if(p == _storeMap.end())
	{
	    return false;
	}
	
	store = (*p).second;
	
	EvictorElementPtr element = store->getIfPinned(ident);
	if(element != 0)
	{
	    assert(!element->stale);    
	    
	    IceUtil::Mutex::Lock lock(element->mutex);
	    return element->status != EvictorElement::dead && 
		element->status != EvictorElement::destroyed;
	}
    }
    return store->dbHasObject(ident);
}

bool
Freeze::EvictorI::hasAnotherFacet(const Identity& ident, const string& facet)
{
    //
    // Must be called with _deactivateController locked.
    //

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
	    ObjectStore* store = (*p).second;
	    
	    bool inCache = false;
	    {
		Lock sync(*this);
		
		EvictorElementPtr element = store->getIfPinned(ident);
		if(element != 0)
		{
		    inCache = true;
		    assert(!element->stale);    
		    
		    IceUtil::Mutex::Lock lock(element->mutex);
		    if(element->status != EvictorElement::dead && 
		       element->status != EvictorElement::destroyed)
		    {
			return true;
		    }
		}
	    }
	    if(!inCache)
	    {
		if(store->dbHasObject(ident))
		{
		    return true;
		}
	    }
	}
    }
    return false;
}
    

ObjectPtr
Freeze::EvictorI::locate(const Current& current, LocalObjectPtr& cookie)
{
    //
    // We need this guard because the application may call locate/finished/deactivate
    // directly.
    //
    DeactivateController::Guard deactivateGuard(_deactivateController);

    //
    // Special ice_ping() handling
    //
    if(current.operation == "ice_ping")
    {
	assert(current.mode == Nonmutating);

	if(hasFacetImpl(current.id, current.facet))
	{
	    if(_trace >= 3)
	    {
		Trace out(_communicator->getLogger(), "Freeze.Evictor");
		out << "ice_ping found \"" << identityToString(current.id)  
		    << "\" with facet \"" << current.facet + "\"";
	    }
	    
	    cookie = 0;
	    return _pingObject;
	}
	else if(hasAnotherFacet(current.id, current.facet))
	{
	    if(_trace >= 3)
	    {
		Trace out(_communicator->getLogger(), "Freeze.Evictor");
		out << "ice_ping raises FacetNotExistException for \"" << identityToString(current.id)  
		    << "\" with facet \"" << current.facet + "\"";
	    }
	    throw FacetNotExistException(__FILE__, __LINE__);
	}
	else
	{
	    if(_trace >= 3)
	    {
		Trace out(_communicator->getLogger(), "Freeze.Evictor");
		out << "ice_ping will raise ObjectNotExistException for \"" << identityToString(current.id)  
		    << "\" with facet \"" << current.facet + "\"";
	    }
	    return 0;
	}
    }
    
    ObjectPtr result = locateImpl(current, cookie);
    
    if(result == 0)
    {
	if(hasAnotherFacet(current.id, current.facet))
	{
	    throw FacetNotExistException(__FILE__, __LINE__);
	}
    }
    return result;
}


ObjectPtr
Freeze::EvictorI::locateImpl(const Current& current, LocalObjectPtr& cookie)
{
    cookie = 0;

    ObjectStore* store = findStore(current.facet);
    if(store == 0)
    {
	return 0;
    }
    
    for(;;)
    {
	EvictorElementPtr element = store->pin(current.id);
	if(element == 0)
	{
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
	if(element->status == EvictorElement::destroyed || element->status == EvictorElement::dead)
	{
	    return 0;
	}

	//
	// It's a good one!
	//
	fixEvictPosition(element);
	element->usageCount++;
	cookie = element;
	assert(element->rec.servant != 0);
	return element->rec.servant;
    }
}

void
Freeze::EvictorI::finished(const Current& current, const ObjectPtr& servant, const LocalObjectPtr& cookie)
{
    assert(servant);

    //
    // If only Ice calls locate/finished/deactivate, then it cannot be deactivated.
    //
    DeactivateController::Guard deactivateGuard(_deactivateController);

    if(cookie != 0)
    {
	EvictorElementPtr element = EvictorElementPtr::dynamicCast(cookie);
	assert(element);
    
	bool enqueue = false;
	
	if(current.mode != Nonmutating)
	{
	    IceUtil::Mutex::Lock lock(element->mutex);
	    
	    if(element->status == EvictorElement::clean)
	    {
		//
		// Assume this operation updated the object
		// 
		element->status = EvictorElement::modified;
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
Freeze::EvictorI::deactivate(const string&)
{
    if(_deactivateController.deactivate())
    {
	try
	{
	    Lock sync(*this);

	    saveNowNoSync();
	    
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
	    
	    if(_watchDogThread != 0)
	    {
		_watchDogThread->terminate();
		_watchDogThread->getThreadControl().join();  
	    }

	    for(StoreMap::iterator p = _storeMap.begin(); p != _storeMap.end(); ++p)
	    {
		(*p).second->close();
	    }
	    
	    _dbEnv = 0;
	    _initializer = 0;
	}
	catch(...)
	{
	    _deactivateController.deactivationComplete();
	    throw;
	}
	_deactivateController.deactivationComplete();
    }
}


void 
Freeze::EvictorI::initialize(const Identity& ident, const string& facet, const ObjectPtr& servant)
{
    if(_initializer != 0)
    {
	_initializer->initialize(_adapter, ident, facet, servant);
    }
}


void
Freeze::EvictorI::run()
{
    try
    {
	for(;;)
	{
	    deque<EvictorElementPtr> allObjects;
	    deque<EvictorElementPtr> deadObjects;

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
	    
	    deque<StreamedObject> streamedObjectQueue;
	    
	    Long streamStart = IceUtil::Time::now().toMilliSeconds();
	    
	    //
	    // Stream each element
	    //
	    for(size_t i = 0; i < size; i++)
	    {
		EvictorElementPtr& element = allObjects[i];
		
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
			case EvictorElement::created:
			case EvictorElement::modified:
			{
			    servant = element->rec.servant;
			    break;
			}   
			case EvictorElement::destroyed:
			{
			    size_t index = streamedObjectQueue.size();
			    streamedObjectQueue.resize(index + 1);
			    StreamedObject& obj = streamedObjectQueue[index];
			    stream(element, streamStart, obj);

			    element->status = EvictorElement::dead;
			    deadObjects.push_back(element);

			    break;
			}   
			case EvictorElement::dead:
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

				if(_watchDogThread != 0)
				{
				    _watchDogThread->activate();
				}
				lockServant.acquire();
				if(_watchDogThread != 0)
				{
				    _watchDogThread->deactivate();
				}

				lockElement.acquire();
				status = element->status;
			    }
  
			    switch(status)
			    {
				case EvictorElement::created:
				case EvictorElement::modified:
				{
				    if(servant == element->rec.servant)
				    {
					size_t index = streamedObjectQueue.size();
					streamedObjectQueue.resize(index + 1);
					StreamedObject& obj = streamedObjectQueue[index];
					stream(element, streamStart, obj);

					element->status = EvictorElement::clean;
				    }
				    else
				    {
					tryAgain = true;
				    }
				    break;
				}
				case EvictorElement::destroyed:
				{
				    lockServant.release();
				    
				    size_t index = streamedObjectQueue.size();
				    streamedObjectQueue.resize(index + 1);
				    StreamedObject& obj = streamedObjectQueue[index];
				    stream(element, streamStart, obj);

				    element->status = EvictorElement::dead;
				    deadObjects.push_back(element);
				    break;
				}   
				case EvictorElement::dead:
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
			    ex.message = string(typeid(*element->rec.servant).name()) 
				+ " does not implement IceUtil::AbstractMutex";
			    throw ex;
			}
		    }
		} while(tryAgain);
	    }
	    
	    if(_trace >= 1)
	    {
		Long now = IceUtil::Time::now().toMilliSeconds();
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
		    
		    Long saveStart = IceUtil::Time::now().toMilliSeconds();
		    try
		    {
			DbTxn* tx = 0;
			_dbEnv->getEnv()->txn_begin(0, &tx, 0);
			try
			{	
			    for(size_t i = 0; i < txSize; i++)
			    {
				StreamedObject& obj = streamedObjectQueue[i];
				obj.store->save(obj.key, obj.value, obj.status, tx);
			    }
			}
			catch(...)
			{
			    tx->abort();
			    throw;
			}
			tx->commit(0);
			streamedObjectQueue.erase
			    (streamedObjectQueue.begin(), 
			     streamedObjectQueue.begin() + txSize);
			
			if(_trace >= 1)
			{
			    Long now = IceUtil::Time::now().toMilliSeconds();
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
			    out << "Deadlock in Freeze::EvictorI::run while writing into Db \"" + _filename
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
		for(deque<EvictorElementPtr>::iterator p = allObjects.begin();
		    p != allObjects.end(); p++)
		{
		    EvictorElementPtr& element = *p;
		    element->usageCount--;
		}
		allObjects.clear();

		for(deque<EvictorElementPtr>::iterator q = deadObjects.begin();
		    q != deadObjects.end(); q++)
		{
		    EvictorElementPtr& element = *q;
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
			    if(element->status == EvictorElement::dead)
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
    catch(const IceUtil::Exception& ex)
    {
	Error out(_communicator->getLogger());
	out << "Saving thread killed by exception: " << ex;
	out.flush();
	handleFatalError(this, _communicator);
    }
    catch(const std::exception& ex)
    {
	Error out(_communicator->getLogger());
	out << "Saving thread killed by std::exception: " << ex.what();
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


const string&
Freeze::EvictorI::filename() const
{
    return _filename;
}

void
Freeze::EvictorI::saveNow()
{
    Lock sync(*this);
    saveNowNoSync();
}

void
Freeze::EvictorI::saveNowNoSync()
{
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
Freeze::EvictorI::evict()
{
    //
    // Must be called with *this locked
    //

    assert(_currentEvictorSize == _evictorList.size());

    list<EvictorElementPtr>::reverse_iterator p = _evictorList.rbegin();
    
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

	EvictorElementPtr& element = *p;
	assert(!element->stale);
	assert(element->keepCount == 0);

	if(_trace >= 2 || (_trace >= 1 && _evictorList.size() % 50 == 0))
	{
	    string facet = element->store.facet();

	    Trace out(_communicator->getLogger(), "Freeze.Evictor");
	    out << "evicting \"" << element->cachePosition->first << "\" ";
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
	p = list<EvictorElementPtr>::reverse_iterator(_evictorList.erase(element->evictPosition));
	_currentEvictorSize--;
    }
}

void 
Freeze::EvictorI::fixEvictPosition(const EvictorElementPtr& element)
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
Freeze::EvictorI::evict(const EvictorElementPtr& element)
{
    assert(!element->stale);
    assert(element->keepCount == 0);

    _evictorList.erase(element->evictPosition);
    _currentEvictorSize--;
    element->stale = true;
    element->store.unpin(element->cachePosition);
}


void
Freeze::EvictorI::addToModifiedQueue(const EvictorElementPtr& element)
{
    element->usageCount++;
    _modifiedQueue.push_back(element);
    
    if(_saveSizeTrigger >= 0 && static_cast<Int>(_modifiedQueue.size()) >= _saveSizeTrigger)
    {
	notifyAll();
    }
}


void
Freeze::EvictorI::stream(const EvictorElementPtr& element, Long streamStart, StreamedObject& obj)
{
    assert(element->status != EvictorElement::dead);
    
    obj.status = element->status;
    obj.store = &element->store;
    
    const Identity& ident = element->cachePosition->first;
    ObjectStore::marshal(ident, obj.key, _communicator);

    if(element->status != EvictorElement::destroyed)
    {
	//
	// Update stats first
	//
	Statistics& stats = element->rec.stats;
	Long diff = streamStart - (stats.creationTime + stats.lastSaveTime);
	if(stats.lastSaveTime == 0)
	{
	    stats.lastSaveTime = diff;
	    stats.avgSaveTime = diff;
	}
	else
	{
	    stats.lastSaveTime = streamStart - stats.creationTime;
	    stats.avgSaveTime = static_cast<Long>(stats.avgSaveTime * 0.95 + diff * 0.05);
	}
	ObjectStore::marshal(element->rec, obj.value, _communicator);
    }
}

Freeze::ObjectStore*
Freeze::EvictorI::findStore(const string& facet) const
{
    Lock sync(*this);
  
    StoreMap::const_iterator p = _storeMap.find(facet);
    if(p == _storeMap.end())
    {
	return 0;
    }
    else
    {
	return (*p).second;
    }
}


vector<string>
Freeze::EvictorI::allDbs() const
{
    vector<string> result;
    
    try
    {
	Db db(_dbEnv->getEnv(), 0);
	db.open(0, _filename.c_str(), 0, DB_UNKNOWN, DB_RDONLY, 0);

	Dbc* dbc = 0;
	db.cursor(0, &dbc, 0);
	
	Dbt dbKey;
	dbKey.set_flags(DB_DBT_MALLOC);
	
	Dbt dbValue;
	dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
	
	bool more = true;
	while(more)
	{
	    more = (dbc->get(&dbKey, &dbValue, DB_NEXT) == 0);
	    if(more)
	    {
		string dbName(static_cast<char*>(dbKey.get_data()), dbKey.get_size());
		
		if(dbName.find(indexPrefix) != 0)
		{
		    result.push_back(dbName);
		}
		free(dbKey.get_data());
	    }
	}
	
	dbc->close();
	db.close(0);
    }
    catch(const DbException& dx)
    {
	if(dx.get_errno() != ENOENT)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
    
    return result;
}


//
// Print for the various exception types.
//

void
Freeze::EvictorDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nevictor deactivated";
}

void
Freeze::NoSuchElementException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno such element";
}

