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

#include <Freeze/EvictorI.h>
#include <Freeze/Initialize.h>
#include <IceUtil/AbstractMutex.h>
#include <Freeze/Util.h>
#include <Freeze/EvictorIteratorI.h>

#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;


string Freeze::EvictorI::defaultDb = "$default";
string Freeze::EvictorI::indexPrefix = "$index:";

Freeze::EvictorPtr
Freeze::createEvictor(const ObjectAdapterPtr& adapter, 
		      const string& envName, 
		      const string& filename,
		      const ServantInitializerPtr& initializer,
		      const vector<IndexPtr>& indices,
		      bool createDb)
{
    return new EvictorI(adapter, envName, filename, initializer, indices, createDb);
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
    return new EvictorI(adapter, envName, dbEnv, filename, initializer, indices, createDb);
}


Freeze::EvictorI::EvictorI(const ObjectAdapterPtr& adapter, 
			   const string& envName, 
			   const string& filename,
			   const ServantInitializerPtr& initializer,
			   const vector<IndexPtr>& indices,
			   bool createDb) :
    _evictorSize(10),
    _currentEvictorSize(0),
    _deactivated(false),
    _adapter(adapter),
    _communicator(adapter->getCommunicator()),
    _initializer(initializer),
    
    _dbEnv(0),
    _dbEnvHolder(SharedDbEnv::get(_communicator, envName)),
    _filename(filename),
    _createDb(createDb),
    _trace(0)
{
    _dbEnv = _dbEnvHolder.get();
    init(envName, indices);
}

Freeze::EvictorI::EvictorI(const ObjectAdapterPtr& adapter, 
			   const string& envName, 
			   DbEnv& dbEnv, 
			   const string& filename, 
			   const ServantInitializerPtr& initializer,
			   const vector<IndexPtr>& indices,
			   bool createDb) :
    
    _evictorSize(10),
    _currentEvictorSize(0),
    _deactivated(false),
    _adapter(adapter),
    _communicator(adapter->getCommunicator()),
    _initializer(initializer),

    _dbEnv(&dbEnv),
    _filename(filename),
    _createDb(createDb),
    _trace(0)
{
    init(envName, indices);
}

void
Freeze::EvictorI::init(const string& envName, const vector<IndexPtr>& indices)
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
    // Start saving thread
    //
    start();
}

Freeze::EvictorI::~EvictorI()
{
    if(!_deactivated)
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
    Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

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

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

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
    ObjectStore* store = 0;
    
    for(;;)
    {
	{
	    Lock sync(*this);
	    
	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }
	    
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
	pair<EvictorElementPtr, bool> ir = store->insert(ident, element);
      
	if(ir.second == false)
	{
	    element = ir.first;
	}

	{
	    Lock sync(*this);

	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }

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
	    ex.id += " -f " + facet;
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

    
    //
    // TODO: there is currently no way to create an ObjectPrx
    // with a facet!
    //
    return _adapter->createProxy(ident);
}

void
Freeze::EvictorI::remove(const Identity& ident)
{
    removeFacet(ident, "");
}

void
Freeze::EvictorI::removeFacet(const Identity& ident, const string& facet)
{
    ObjectStore* store = findStore(facet);
    bool notThere = (store == 0);

    if(store != 0)
    {
	for(;;)
	{
	    //
	    // Retrieve object
	    //
	    
	    EvictorElementPtr element = store->pin(ident);
	    if(element == 0)
	    {
		notThere = true;
	    }
	    else
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
			element->status = EvictorElement::destroyed;
			element->rec.servant = 0;
			addToModifiedQueue(element);
			break;
		    }
		    case EvictorElement::created:
		    {
			element->status = EvictorElement::dead;
			element->rec.servant = 0;
			break;
		    }
		    case EvictorElement::modified:
		    {
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
			notThere = true;
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
    }
    
    if(notThere)
    {
	NotRegisteredException ex(__FILE__, __LINE__);
	ex.kindOfObject = "servant";
	ex.id = identityToString(ident);
	if(!facet.empty())
	{
	    ex.id += " -f " + facet;
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
}

EvictorIteratorPtr
Freeze::EvictorI::getIterator(const string& facet, Int batchSize)
{
    ObjectStore* store = 0;
    {
	Lock sync(*this);
	if(_deactivated)
	{
	    throw EvictorDeactivatedException(__FILE__, __LINE__);
	}
	
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
    ObjectStore* store = 0;

    {
	Lock sync(*this);
	
	if(_deactivated)
	{
	    throw EvictorDeactivatedException(__FILE__, __LINE__);
	}
	
	StoreMap::iterator p = _storeMap.find(facet);
	if(p == _storeMap.end())
	{
	    return false;
	}
	
	ObjectStore* store = (*p).second;
	
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


ObjectPtr
Freeze::EvictorI::locate(const Current& current, LocalObjectPtr& cookie)
{
    ObjectPtr result = locateImpl(current, cookie);
    
    if(result == 0)
    {
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
	    // Do not check again the current facet
	    //
	    if((*p).first != current.facet)
	    { 
		ObjectStore* store = (*p).second;

		bool inCache = false;
		{
		    Lock sync(*this);
		    
		    EvictorElementPtr element = store->getIfPinned(current.id);
		    if(element != 0)
		    {
			inCache = true;
			assert(!element->stale);    
			
			IceUtil::Mutex::Lock lock(element->mutex);
			if(element->status != EvictorElement::dead && 
			   element->status != EvictorElement::destroyed)
			{
			    throw FacetNotExistException(__FILE__, __LINE__);
			}
		    }
		}
		if(!inCache)
		{
		    if(store->dbHasObject(current.id))
		    {
			throw FacetNotExistException(__FILE__, __LINE__);
		    }
		}
	    }
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
	assert(!_deactivated);

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
	else
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
    Lock sync(*this);
    
    //
    // TODO: wait until all outstanding requests have completed
    //

    if(!_deactivated)
    { 	
	if(_trace >= 1)
	{
	    Trace out(_communicator->getLogger(), "Freeze.Evictor");
	    out << "deactivating, saving unsaved Ice objects to the database";
	}

	saveNowNoSync();
	
	//
	// Set the evictor size to zero, meaning that we will evict
	// everything possible.
	//
	_evictorSize = 0;
	evict();
	
	_deactivated = true;
	notifyAll();
	sync.release();
	getThreadControl().join();

	for(StoreMap::iterator p = _storeMap.begin(); p != _storeMap.end(); ++p)
	{
	    (*p).second->close();
	}

	_dbEnv = 0;
	_dbEnvHolder = 0;
	_initializer = 0;
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
		while(!_deactivated &&
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
		
		if(_deactivated)
		{
		    assert(_modifiedQueue.size() == 0);
		    if(saveNowThreadsSize > 0)
		    {
			_saveNowThreads.clear();
			notifyAll();
		    }
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
				lockServant.acquire();
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
			_dbEnv->txn_begin(0, &tx, 0);
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
			if(element->usageCount == 0)
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
	::abort();
    }
    catch(const std::exception& ex)
    {
	Error out(_communicator->getLogger());
	out << "Saving thread killed by std::exception: " << ex.what();
	out.flush();
	::abort();
    }
    catch(...)
    {
	Error out(_communicator->getLogger());
	out << "Saving thread killed by unknown exception";
	out.flush();
	::abort();
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

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

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

void 
Freeze::EvictorI::evict(const EvictorElementPtr& element)
{
    assert(!element->stale);
 
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
    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

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
	Db db(_dbEnv, 0);
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

