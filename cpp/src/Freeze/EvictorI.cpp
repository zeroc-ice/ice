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

#include <Ice/Object.h> // Not included in Ice/Ice.h
#include <Freeze/EvictorI.h>
#include <Freeze/IdentityObjectRecordDict.h>
#include <Freeze/Initialize.h>
#include <sys/stat.h>
#include <IceUtil/AbstractMutex.h>
#include <typeinfo>

using namespace std;
using namespace Freeze;
using namespace Ice;

#ifdef _WIN32
#   define FREEZE_DB_MODE 0
#else
#   define FREEZE_DB_MODE (S_IRUSR | S_IWUSR)
#endif

namespace Freeze
{

class EvictorIteratorI : public EvictorIterator
{
public:

    EvictorIteratorI(Db&, const CommunicatorPtr&);

    virtual ~EvictorIteratorI();

    virtual bool hasNext();
    virtual Ice::Identity next();
    virtual void destroy();

private:

    Dbc* _dbc;
    Ice::Identity _current;
    bool _currentSet;
    CommunicatorPtr _communicator;
    Key _key;
};

}


namespace
{

//
// Streamed objects
//
struct StreamedObject
{
    Key key;
    Value value;
    Ice::Byte status;
};

inline void 
initializeDbt(vector<Ice::Byte>& v, Dbt& dbt)
{
    dbt.set_data(&v[0]);
    dbt.set_size(v.size());
    dbt.set_ulen(v.size());
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

}

Freeze::EvictorPtr
Freeze::createEvictor(const Ice::CommunicatorPtr& communicator, 
		      const std::string& envName, 
		      const std::string& dbName,
		      bool createDb)
{
    return new EvictorI(communicator, envName, dbName, createDb);
}

Freeze::EvictorPtr
Freeze::createEvictor(const Ice::CommunicatorPtr& communicator, 
		      DbEnv& dbEnv, 
		      const std::string& dbName,
		      bool createDb)
{
    return new EvictorI(communicator, dbEnv, dbName, createDb);
}



Freeze::EvictorI::EvictorI(const Ice::CommunicatorPtr communicator, 
			   const std::string& envName, 
			   const std::string& dbName, 
			   bool createDb) :
    _evictorSize(10),
    _deactivated(false),
    _communicator(communicator),
    _dbEnv(0),
    _dbEnvHolder(SharedDbEnv::get(communicator, envName)),
    _trace(0),
    _noSyncAllowed(false)
{
    _dbEnv = _dbEnvHolder.get();
    init(envName, dbName, createDb);
}

Freeze::EvictorI::EvictorI(const Ice::CommunicatorPtr communicator, 
			   DbEnv& dbEnv, 
			   const std::string& dbName, 
			   bool createDb) :
    _evictorSize(10),
    _deactivated(false),
    _communicator(communicator),
    _dbEnv(&dbEnv),
    _trace(0),
    _noSyncAllowed(false)
{
    init("Extern", dbName, createDb);
}

void
Freeze::EvictorI::init(const string& envName, const string& dbName, bool createDb)
{
    _trace = _communicator->getProperties()->getPropertyAsInt("Freeze.Trace.Evictor");
    _noSyncAllowed = (_communicator->getProperties()->getPropertyAsInt("Freeze.Evictor.NoSyncAllowed") != 0);

    string propertyPrefix = string("Freeze.Evictor.") + envName + '.' + dbName; 
    
    //
    // By default, we save every minute or when the size of the modified queue
    // reaches 10.
    //

    _saveSizeTrigger = _communicator->getProperties()->
	getPropertyAsIntWithDefault(propertyPrefix + ".SaveSizeTrigger", 10);

    Int savePeriod = _communicator->getProperties()->
	getPropertyAsIntWithDefault(propertyPrefix + ".SavePeriod", 60 * 1000);

    _savePeriod = IceUtil::Time::milliSeconds(savePeriod);
   
    try
    {
	_db.reset(new Db(_dbEnv, 0));
	u_int32_t flags = DB_AUTO_COMMIT | DB_THREAD;
	if(createDb)
	{
	    flags |= DB_CREATE;
	}
	_db->open(0, dbName.c_str(), 0, DB_BTREE, flags, FREEZE_DB_MODE);
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }

    _lastSave = IceUtil::Time::now();

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
    _evictorSize = static_cast<EvictorMap::size_type>(evictorSize);

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
Freeze::EvictorI::createObject(const Identity& ident, const ObjectPtr& servant)
{
    Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    EvictorMap::iterator p = _evictorMap.find(ident);

    if(p != _evictorMap.end())
    {
	EvictorElementPtr& element = p->second;

	{
	    IceUtil::Mutex::Lock lockRec(element->mutex);
	    
	    switch(element->status)
	    {
		case clean:
		{
		    element->status = modified;
		    addToModifiedQueue(p, element);
		    break;
		}
		case created:
		case modified:
		{
		    //
		    // Nothing to do.
		    // No need to push it on the modified queue as a created resp
		    // modified element is either already on the queue or about 
		    // to be saved. When saved, it becomes clean.
		    //
		    break;
		}  
		case destroyed:
		{
		    element->status = modified;
		    //
		    // No need to push it on the modified queue, as a destroyed element
		    // is either already on the queue or about to be saved. When saved,
		    // it becomes dead.
		    //
		    break;
		}
		case dead:
		{
		    element->status = created;
		    addToModifiedQueue(p, element);
		    break;
		}
		default:
		{
		    assert(0);
		    break;
		}
	    }

	    element->rec.servant = servant;
	}
	_evictorList.erase(element->position);
	_evictorList.push_front(p);
	element->position = _evictorList.begin();
    }
    else
    {
	//
	// Create a new object
	//
	
	ObjectRecord rec;
	rec.servant = servant;
	rec.stats.creationTime = IceUtil::Time::now().toMilliSeconds();
	rec.stats.lastSaveTime = 0;
	rec.stats.avgSaveTime = 0;
	
	//
	// Add an Ice object with its servant to the evictor queue.
	//
	
	EvictorElementPtr element = new EvictorElement;
	element->rec = rec;
	element->usageCount = 0;
	element->status = created;
	
	pair<EvictorMap::iterator, bool> pair = _evictorMap.insert(EvictorMap::value_type(ident, element));
	assert(pair.second);

	_evictorList.push_front(pair.first);
	element->position = _evictorList.begin();
	
	addToModifiedQueue(pair.first, element);

	//
	// Evict as many elements as necessary.
	//
	evict();
    } 

    sync.release();
      
    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Evictor");
	out << "created \"" << ident << "\"";
    }
}

void
Freeze::EvictorI::destroyObject(const Identity& ident)
{
    Lock sync(*this);
    
    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    EvictorMap::iterator p = _evictorMap.find(ident);
    if(p != _evictorMap.end())
    {
	EvictorElementPtr& element = p->second;

	IceUtil::Mutex::Lock lockRec(element->mutex);
	    
	switch(element->status)
	{
	    case clean:
	    {
		element->status = destroyed;
		addToModifiedQueue(p, element);
		break;
	    }
	    case created:
	    {
		element->status = dead;
		break;
	    }
	    case modified:
	    {
		element->status = destroyed;
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
		//
		// Nothing to do!
		//
		break;
	    }
	    default:
	    {
		assert(0);
		break;
	    }
	}
    }
    else
    {
	//
	// Set a real ObjectRecord in case this object gets recreated
	//
	ObjectRecord rec;
	rec.servant = 0;
	rec.stats.creationTime = IceUtil::Time::now().toMilliSeconds();
	rec.stats.lastSaveTime = 0;
	rec.stats.avgSaveTime = 0;

	EvictorElementPtr element = new EvictorElement;
	element->rec = rec;
	element->usageCount = 0;
	element->status = destroyed;

	pair<EvictorMap::iterator, bool> pair = _evictorMap.insert(EvictorMap::value_type(ident, element));
	assert(pair.second);
	element->position = _evictorList.insert(_evictorList.end(), pair.first);

	addToModifiedQueue(pair.first, element);

	evict();
    }
    
    sync.release();

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Evictor");
	out << "destroyed \"" << ident << "\"";
    }
}

void
Freeze::EvictorI::installServantInitializer(const ServantInitializerPtr& initializer)
{
    Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    _initializer = initializer;
}

EvictorIteratorPtr
Freeze::EvictorI::getIterator()
{
    {
	Lock sync(*this);
	if(_deactivated)
	{
	    throw EvictorDeactivatedException(__FILE__, __LINE__);
	}
	saveNowNoSync();
    }
    
    return new EvictorIteratorI(*_db, _communicator);
}

bool
Freeze::EvictorI::hasObject(const Ice::Identity& ident)
{
    Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    EvictorMap::iterator p = _evictorMap.find(ident);
    
    if(p != _evictorMap.end())
    {
	EvictorElementPtr& element = p->second;
	IceUtil::Mutex::Lock lockRec(element->mutex);
	return (element->status != destroyed && element->status != dead);
    }
    else
    {
	//
	// Release sync to increase concurrency
	//
	sync.release();

	return dbHasObject(ident);
    }
}


ObjectPtr
Freeze::EvictorI::locate(const Current& current, LocalObjectPtr& cookie)
{
    ObjectRecord rec;
    bool objectLoaded = false;

    for(;;)
    {
	EvictorMap::iterator p;
	bool objectFound = false;

	{
	    Lock sync(*this);

	    // 
	    // If this operation is called on a deactivated servant locator,
	    // it's a bug in Ice.
	    //
	    assert(!_deactivated);
	    
	    p = _evictorMap.find(current.id);
	    objectFound = (p != _evictorMap.end());
	    
	    if(objectFound)
	    {
		//
		// Ice object found in evictor map. Push it to the front of
		// the evictor list, so that it will be evicted last.
		//
		EvictorElementPtr& element = p->second;
		_evictorList.erase(element->position);
		_evictorList.push_front(p);
		element->position = _evictorList.begin();
		element->usageCount++;
		cookie = element;

		//
		// Later (after releasing the mutex), check that this
		// object is not dead or destroyed
		//
	    }
	    else if(objectLoaded)
	    {
		//
		// Proceed with the object loaded in the previous loop
		//
		
		//
		// If an initializer is installed, call it now.
		//
		if(_initializer)
		{
		    _initializer->initialize(current.adapter, current.id, rec.servant);
		}
		    
		//
		// Add an Ice object with its servant to the evictor queue.
		//
		
		EvictorElementPtr element = new EvictorElement;
		element->rec = rec;
		element->usageCount = 1;
		element->status = clean;
		
		pair<EvictorMap::iterator, bool> pair = _evictorMap.insert(
		    EvictorMap::value_type(current.id, element));
		assert(pair.second);
		_evictorList.push_front(pair.first);
		element->position = _evictorList.begin();
		
		cookie = element;
		
		//
		// Evict as many elements as necessary.
		//
		evict();
		
		return rec.servant;
	    }
	    //
	    // Else fall to the after-sync processing
	    //
	}
	
	if(objectFound)
	{
	    EvictorElementPtr& element = p->second;

	    if(_trace >= 2)
	    {
		Trace out(_communicator->getLogger(), "Evictor");
		out << "found \"" << current.id << "\" in the queue";
	    }
	    
	    //
	    // Return servant if object not dead or destroyed
	    //
	    {
		IceUtil::Mutex::Lock lockRec(element->mutex);
		if(element->status != destroyed && element->status != dead)
		{
		    return element->rec.servant;
		}
	    }
	
	    //
	    // Object is destroyed or dead: clean-up
	    //
	    if(_trace >= 2)
	    {
		Trace out(_communicator->getLogger(), "Evictor");
		out << "\"" << current.id << "\" was dead or destroyed";
	    }
	    Lock sync(*this);
	    element->usageCount--;
	    return 0;	  
	}
	else
	{
	    //
	    // Load object now and loop
	    //
		    
	    if(_trace >= 2)
	    {
		Trace out(_communicator->getLogger(), "Evictor");
		out << "couldn't find \"" << current.id << "\" in the queue\n"
		    << "loading \"" << current.id << "\" from the database";
	    }
	    
	    if(getObject(current.id, rec))
	    {
		objectLoaded = true;
		
		//
		// Loop
		//
	    }
	    else
	    {
		//
		// The Ice object with the given identity does not exist,
		// client will get an ObjectNotExistException.
		//
		return 0;
	    }
	}
    }
}

void
Freeze::EvictorI::finished(const Current& current, const ObjectPtr& servant, const LocalObjectPtr& cookie)
{
    assert(servant);

    EvictorElementPtr element = EvictorElementPtr::dynamicCast(cookie);
    assert(element);

    bool enqueue = false;

    if(current.mode != Nonmutating)
    {
	IceUtil::Mutex::Lock lockRec(element->mutex);

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

    assert(!_deactivated);

    //
    // Decrease the usage count of the evictor queue element.
    //
    assert(element->usageCount >= 1);
    --element->usageCount;

    if(enqueue)
    {
	EvictorMap::iterator p = _evictorMap.find(current.id);
	assert(p != _evictorMap.end());
	
	addToModifiedQueue(p, element);
    }
    else
    {
	//
	// Evict as many elements as necessary.
	//
	evict();
    }
}

void
Freeze::EvictorI::deactivate(const string&)
{
    Lock sync(*this);
	
    if(!_deactivated)
    {
	saveNowNoSync();
       	
	if(_trace >= 1)
	{
	    Trace out(_communicator->getLogger(), "Evictor");
	    out << "deactivating, saving unsaved Ice objects to the database";
	}
	
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

	try
	{
	    _db->close(0);
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	_db.reset();
	_dbEnv = 0;
	_dbEnvHolder = 0;
    }
}

void
Freeze::EvictorI::run()
{
    for(;;)
    {
    	deque<EvictorMap::iterator> allObjects;
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
    
	//
	// Usage count release
	//
	deque<EvictorElementPtr> releaseAfterStreaming;
	deque<EvictorElementPtr> releaseAfterCommit;
	
	deque<StreamedObject> streamedObjectQueue;
	
	Long saveStart = IceUtil::Time::now().toMilliSeconds();
	
	//
	// Stream each element
	//
	for(size_t i = 0; i < size; i++)
	{
	    EvictorElementPtr& element = allObjects[i]->second;
	    
	    IceUtil::Mutex::Lock lockRec(element->mutex);
	    ObjectRecord& rec = element->rec;
	    
	    bool streamIt = true;
	    Ice::Byte status = element->status;
	    switch(status)
	    {
		case created:
		{
		    element->status = clean;
		    releaseAfterCommit.push_back(element);
		    break;
		}   
		case modified:
		{
		    element->status = clean;
		    releaseAfterStreaming.push_back(element);
		    break;
		}
		case destroyed:
		{
		    element->status = dead;
		    releaseAfterCommit.push_back(element);
		    break;
		}   
		default:
		{
		    //
		    // Nothing to do (could be a duplicate)
		    //
		    streamIt = false;
		    releaseAfterStreaming.push_back(element);
		    break;
		}
	    }
	    
	    if(streamIt)
	    {
		size_t index = streamedObjectQueue.size();
		streamedObjectQueue.resize(index + 1);
		StreamedObject& obj = streamedObjectQueue[index];
		IdentityObjectRecordDictKeyCodec::write(allObjects[i]->first, obj.key, _communicator);
		obj.status = status;
		if(status != destroyed)
		{
		    IceUtil::AbstractMutex* mutex = dynamic_cast<IceUtil::AbstractMutex*>(rec.servant.get());
		    if(mutex != 0)
		    {
			IceUtil::AbstractMutex::Lock lockServant(*mutex);
			writeObjectRecordToValue(saveStart, rec, obj.value);
		    }
		    else
		    {
			if(_noSyncAllowed)
			{
			    writeObjectRecordToValue(saveStart, rec, obj.value);
			}
			else
			{
			    DBException ex(__FILE__, __LINE__);
			    ex.message = string(typeid(*rec.servant).name()) 
				+ " does not implement IceUtil::AbstractMutex and Freeze.Evictor.NoSyncAllowed is 0";
			    throw ex;
			}
		    }
		}
	    }    
	}
	
	allObjects.clear();
	
	if(releaseAfterStreaming.size() > 0)
	{
	    Lock sync(*this);
	    for(deque<EvictorElementPtr>::iterator q = releaseAfterStreaming.begin();
		q != releaseAfterStreaming.end(); q++)
	    {    
		(*q)->usageCount--;
	    }
	    releaseAfterStreaming.clear();
	}	    
	
	//
	// Now let's save all these streamed objects to disk using a transaction
	//
	
	//
	// Each time we get a deadlock, we reduce the number of objects to save
	// per transaction
	//
	size_t txSize = streamedObjectQueue.size();
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
		
		try
		{
		    DbTxn* tx = 0;
		    _dbEnv->txn_begin(0, &tx, 0);
		    try
		    {   
			for(size_t i = 0; i < txSize; i++)
			{
			    StreamedObject& obj = streamedObjectQueue[i];
			    if(obj.status == destroyed)
			    {
				//
				// May not exist in the database
				//
				
				Dbt dbKey;
				initializeDbt(obj.key, dbKey);
				int err = _db->del(tx, &dbKey, 0);
				if(err != 0 && err != DB_NOTFOUND)
				{
				    //
				    // Bug in Freeze
				    //
				    throw DBException(__FILE__, __LINE__);
				}
				
			    }
			    else
			    {
				//
				// We can't use NOOVERWRITE as some 'created' objects may
				// actually be already in the database
				//
				
				Dbt dbKey;
				Dbt dbValue;
				initializeDbt(obj.key, dbKey);
				initializeDbt(obj.value, dbValue);
				int err = _db->put(tx, &dbKey, &dbValue, 0);
				if(err != 0)
				{
				    //
				    // Bug in Freeze
				    //
				    throw DBException(__FILE__, __LINE__);
				}
			    }
			}
		    }
		    catch(...)
		    {
			tx->abort();
			throw;
		    }
		    tx->commit(0);
		    streamedObjectQueue.erase(streamedObjectQueue.begin(), streamedObjectQueue.begin() + txSize);
		    
		    if(_trace >= 2)
		    {
			Long now = IceUtil::Time::now().toMilliSeconds();
			Trace out(_communicator->getLogger(), "Evictor");
			out << "saved " << txSize << " objects in " 
			    << static_cast<Int>(now - saveStart) << " ms";
			saveStart = now;
		    }
		}
		catch(const ::DbDeadlockException&)
		{
		    tryAgain = true;
		    txSize = (txSize + 1)/2;
		}
		catch(const ::DbException& dx)
		{
		    DBException ex(__FILE__, __LINE__);
		    ex.message = dx.what();
		    throw ex;
		}
	} 
	} while(tryAgain);
	
	
	{
	    Lock sync(*this);
	    
	    for(deque<EvictorElementPtr>::iterator q = releaseAfterCommit.begin();
		q != releaseAfterCommit.end(); q++)
	    {
		(*q)->usageCount--;
	    }
	    releaseAfterCommit.clear();
	    
	    if(saveNowThreadsSize > 0)
	    {
		_saveNowThreads.erase(_saveNowThreads.begin(), _saveNowThreads.begin() + saveNowThreadsSize);
		notifyAll();
	    }
	}
	_lastSave = IceUtil::Time::now();
    }
}
    

void
Freeze::EvictorI::evict()
{
    list<EvictorMap::iterator>::reverse_iterator p = _evictorList.rbegin();

    //
    // With most STL implementations, _evictorMap.size() is faster
    // than _evictorList.size().
    //
    while(_evictorMap.size() > _evictorSize)
    {
	//
	// Get the last unused element from the evictor queue.
	//
	EvictorMap::iterator q;
	while(p != _evictorList.rend())
	{
	    q = *p;
	    if(q->second->usageCount == 0)
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

	if(_trace >= 2)
	{
	    Trace out(_communicator->getLogger(), "Evictor");
	    out << "evicting \"" << q->first << "\" from the queue\n"
		<< "number of elements in the queue: " << _evictorMap.size();
	}

	EvictorElementPtr& element = q->second;
	//
	// Remove last unused element from the evictor queue.
	//
	assert(--(p.base()) == element->position);
	p = list<EvictorMap::iterator>::reverse_iterator(_evictorList.erase(element->position));
	_evictorMap.erase(q);
    }
}

bool
Freeze::EvictorI::dbHasObject(const Ice::Identity& ident)
{
    Key key;
    IdentityObjectRecordDictKeyCodec::write(ident, key, _communicator);
    Dbt dbKey;
    initializeDbt(key, dbKey);
    
    //
    // Keep 0 length since we're not interested in the data
    //
    Dbt dbValue;
    dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

    for(;;)
    {
	try
	{
	    int err = _db->get(0, &dbKey, &dbValue, 0);
	    
	    if(err == 0)
	    {
		return true;
	    }
	    else if(err == DB_NOTFOUND)
	    {
		return false;
	    }
	    else
	    {
		assert(0);
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbDeadlockException&)
	{
	    //
	    // Ignored, try again
	    //
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

bool
Freeze::EvictorI::getObject(const Ice::Identity& ident, ObjectRecord& rec)
{
    Key key;
    IdentityObjectRecordDictKeyCodec::write(ident, key, _communicator);
    Dbt dbKey;
    initializeDbt(key, dbKey);

    const size_t defaultValueSize = 1024;
    Value value(defaultValueSize);

    Dbt dbValue;
    initializeDbt(value, dbValue);

    for(;;)
    {
	try
	{
	    int err = _db->get(0, &dbKey, &dbValue, 0);
	    
	    if(err == 0)
	    {
		value.resize(dbValue.get_size());
		IdentityObjectRecordDictValueCodec::read(rec, value, _communicator);
		return true;
	    }
	    else if(err == DB_NOTFOUND)
	    {
		return false;
	    }
	    else
	    {
		assert(0);
		throw DBException(__FILE__, __LINE__);
	    }
	}
	catch(const ::DbMemoryException dx)
	{
	    if(dbValue.get_size() > dbValue.get_ulen())
	    {
		//
		// Let's resize value
		//
		value.resize(dbValue.get_size());
		initializeDbt(value, dbValue);
	    }
	    else
	    {
		//
		// Real problem
		//
		DBException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	}
	catch(const ::DbDeadlockException&)
	{
	    //
	    // Ignored, try again
	    //
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void
Freeze::EvictorI::addToModifiedQueue(const Freeze::EvictorI::EvictorMap::iterator& p,
				     const Freeze::EvictorI::EvictorElementPtr& element)
{
    element->usageCount++;
    _modifiedQueue.push_back(p);
    
    if(_saveSizeTrigger >= 0 && static_cast<Int>(_modifiedQueue.size()) >= _saveSizeTrigger)
    {
	notifyAll();
    }
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
    } while(find(_saveNowThreads.begin(), _saveNowThreads.end(), myself) != _saveNowThreads.end());
}

void
Freeze::EvictorI::writeObjectRecordToValue(Long saveStart, ObjectRecord& rec, Value& value)
{
    //
    // Update stats first
    //
    Statistics& stats = rec.stats;
    Long diff = saveStart - (stats.creationTime + stats.lastSaveTime);
    if(stats.lastSaveTime == 0)
    {
	stats.lastSaveTime = diff;
	stats.avgSaveTime = diff;
    }
    else
    {
	stats.lastSaveTime = saveStart - stats.creationTime;
	stats.avgSaveTime = static_cast<Long>(stats.avgSaveTime * 0.95 + diff * 0.05);
    }
    IdentityObjectRecordDictValueCodec::write(rec, value, _communicator);
}


Freeze::EvictorIteratorI::EvictorIteratorI(Db& db, const CommunicatorPtr& communicator) :
    _dbc(0),
    _currentSet(false),
    _communicator(communicator),
    _key(1024)
{
    try
    {
	db.cursor(0, &_dbc, 0);
    }
    catch(const ::DbException& dx)
    {
	DBException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

Freeze::EvictorIteratorI::~EvictorIteratorI()
{
    if(_dbc != 0)
    {
	destroy();
    }
}

bool
Freeze::EvictorIteratorI::hasNext()
{
    if(_dbc == 0)
    {
	throw Freeze::IteratorDestroyedException(__FILE__, __LINE__);
    }

    if(_currentSet)
    {
	return true;
    }
    else
    {
	if(_key.size() < _key.capacity())
	{
	    _key.resize(_key.capacity());
	}
	Dbt dbKey;
	initializeDbt(_key, dbKey);
	
	//
	// Keep 0 length since we're not interested in the data
	//
	Dbt dbValue;
	dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
	
	for(;;)
	{
	    try
	    {
		if(_dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
		{
		    _key.resize(dbKey.get_size());
		    IdentityObjectRecordDictKeyCodec::read(_current, _key, _communicator);
		    _currentSet = true;
		    return true;
		}
		else
		{
		    return false;
		}
	    }
	    catch(const ::DbMemoryException dx)
	    {
		if(dbKey.get_size() > dbKey.get_ulen())
		{
		    //
		    // Let's resize _key
		    //
		    _key.resize(dbKey.get_size());
		    initializeDbt(_key, dbKey);
		}
		else
		{
		    //
		    // Real problem
		    //
		    DBException ex(__FILE__, __LINE__);
		    ex.message = dx.what();
		    throw ex;
		}
	    }
	    catch(const ::DbDeadlockException& dx)
	    {
		DBDeadlockException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	    catch(const ::DbException& dx)
	    {
		DBException ex(__FILE__, __LINE__);
		ex.message = dx.what();
		throw ex;
	    }
	}
    }
}

Ice::Identity
Freeze::EvictorIteratorI::next()
{
    if(hasNext())
    {
	_currentSet = false;
	return _current;
    }
    else
    {
	throw Freeze::NoSuchElementException(__FILE__, __LINE__);
    }
}

void
Freeze::EvictorIteratorI::destroy()
{
    if(_dbc == 0)
    {
	throw Freeze::IteratorDestroyedException(__FILE__, __LINE__);
    }
    else
    {
	try
	{
	    _dbc->close();
	}
	catch(const ::DbDeadlockException&)
	{
	    // Ignored
	}
	catch(const ::DbException& dx)
	{
	    DBException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	_dbc = 0;
    }
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

void
Freeze::IteratorDestroyedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\niterator destroyed";
}
