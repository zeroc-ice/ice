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

#include <Freeze/EvictorI.h>
#include <Freeze/Initialize.h>
#include <IceUtil/AbstractMutex.h>
#include <sys/stat.h>
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

    EvictorIteratorI(EvictorI&, Int, bool);

    virtual bool hasNext();
    virtual Identity next();

private:

    vector<Identity>::const_iterator
    nextBatch();

    EvictorI& _evictor;
    Int _batchSize;
    bool _loadServants;
    vector<Identity>::const_iterator _batchIterator;

    Key _key;
    Value _value;
    vector<Identity> _batch;
    bool _more;
};

}

namespace
{

inline void 
initializeInDbt(const vector<Byte>& v, Dbt& dbt)
{
    dbt.set_data(const_cast<Byte*>(&v[0]));
    dbt.set_size(v.size());
    dbt.set_ulen(0);
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

inline void 
initializeOutDbt(vector<Byte>& v, Dbt& dbt)
{
    v.resize(v.capacity());
    dbt.set_data(&v[0]);
    dbt.set_size(0);
    dbt.set_ulen(v.size());
    dbt.set_dlen(0);
    dbt.set_doff(0);
    dbt.set_flags(DB_DBT_USERMEM);
}

void
handleMemoryException(const DbMemoryException& dx, Key& key, Dbt& dbKey)
{
    if(dbKey.get_size() > dbKey.get_ulen())
    {
	//
	// Keep the old key size in case it's used as input
	//
	size_t oldKeySize = key.size();

	key.resize(dbKey.get_size());
	initializeOutDbt(key, dbKey);
	dbKey.set_size(oldKeySize);
    }
    else
    {
	//
	// Real problem
	//
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

void
handleMemoryException(const DbMemoryException& dx, Key& key, Dbt& dbKey, Value& value, Dbt& dbValue)
{
    bool resized = false;
    if(dbKey.get_size() > dbKey.get_ulen())
    {
	size_t oldKeySize = key.size();
	key.resize(dbKey.get_size());
	initializeOutDbt(key, dbKey);
	dbKey.set_size(oldKeySize);
	resized = true;
    }
    
    if(dbValue.get_size() > dbValue.get_ulen())
    {
	value.resize(dbValue.get_size());
	initializeOutDbt(value, dbValue);
	resized = true;
    }
    
    if(!resized)
    {
	//
	// Real problem
	//
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
}

inline bool startWith(Key key, Key root)
{
    if(root.size() > key.size())
    {
	return false;
    }
    return memcmp(&root[0], &key[0], root.size()) == 0;
}

//
// Marshaling/unmarshaling persistent (key, data) pairs. The marshalRoot function
// is used to create a key prefix containing only the key's identity.
//

void
marshalRoot(const Identity& v, Key& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    v.__write(&stream);
    bytes.swap(stream.b);
}

void
marshal(const EvictorStorageKey& v, Key& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    v.__write(&stream);
    bytes.swap(stream.b);
}

void
unmarshal(EvictorStorageKey& v, const Key& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.b = bytes;
    stream.i = stream.b.begin();
    v.__read(&stream);
}

void
marshal(const ObjectRecord& v, Value& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.marshalFacets(false);
    v.__write(&stream);
    stream.writePendingObjects();
    bytes.swap(stream.b);
}

void
unmarshal(ObjectRecord& v, const Value& bytes, const CommunicatorPtr& communicator)
{
    IceInternal::InstancePtr instance = IceInternal::getInstance(communicator);
    IceInternal::BasicStream stream(instance.get());
    stream.b = bytes;
    stream.i = stream.b.begin();
    v.__read(&stream);
    stream.readPendingObjects();
}

}

Freeze::EvictorPtr
Freeze::createEvictor(const CommunicatorPtr& communicator, 
		      const string& envName, 
		      const string& dbName,
		      bool createDb)
{
    return new EvictorI(communicator, envName, dbName, createDb);
}

Freeze::EvictorPtr
Freeze::createEvictor(const CommunicatorPtr& communicator, 
		      const string& envName, 
		      DbEnv& dbEnv, 
		      const string& dbName,
		      bool createDb)
{
    return new EvictorI(communicator, envName, dbEnv, dbName, createDb);
}



Freeze::EvictorI::EvictorI(const CommunicatorPtr communicator, 
			   const string& envName, 
			   const string& dbName, 
			   bool createDb) :
    _evictorSize(10),
    _deactivated(false),
    _communicator(communicator),
    _dbEnv(0),
    _dbEnvHolder(SharedDbEnv::get(communicator, envName)),
    _trace(0),
    _noSyncAllowed(false),
    _generation(0)
{
    _dbEnv = _dbEnvHolder.get();
    init(envName, dbName, createDb);
}

Freeze::EvictorI::EvictorI(const CommunicatorPtr communicator, 
			   const string& envName, 
			   DbEnv& dbEnv, 
			   const string& dbName, 
			   bool createDb) :
    _evictorSize(10),
    _deactivated(false),
    _communicator(communicator),
    _dbEnv(&dbEnv),
    _trace(0),
    _noSyncAllowed(false),
    _generation(0)
{
    init(envName, dbName, createDb);
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
   
    //
    // By default, we save at most 10 * SaveSizeTrigger objects per transaction
    //
    _maxTxSize = _communicator->getProperties()->
	getPropertyAsIntWithDefault(propertyPrefix + ".MaxTxSize", 10 * _saveSizeTrigger);
    
    if(_maxTxSize <= 0)
    {
	_maxTxSize = 100;
    }	

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
    catch(const DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
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
    EvictorElementPtr loadedElement = 0;
    int loadedElementGeneration = 0;
    bool triedToLoadElement = false;

    for(;;)
    {
	{
	    Lock sync(*this);
	    
	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }
	    
	    EvictorMap::iterator p = _evictorMap.find(ident);
	   
	    if(p == _evictorMap.end() && triedToLoadElement)
	    {
		if(loadedElementGeneration == _generation)
		{
		    if(loadedElement != 0)
		    {
			p = insertElement(0, ident, loadedElement);
		    }
		}
		else
		{
		    loadedElement = 0;
		    triedToLoadElement = false;
		}
	    }
	    
	    bool replacing = (p != _evictorMap.end());

	    if(replacing || triedToLoadElement)
	    {
		if(replacing)
		{
		    EvictorElementPtr& element = p->second;
		    
		    //
		    // Destroy all existing facets
		    //
		    for(FacetMap::iterator q = element->facets.begin(); q != element->facets.end(); q++)
		    {
			destroyFacetImpl(q, q->second);
		    }
		}
		else
		{
		    //
		    // Let's insert an empty EvitorElement
		    //
		    EvictorElementPtr element = new EvictorElement;
		    
		    pair<EvictorMap::iterator, bool> pair = _evictorMap.insert(EvictorMap::value_type(ident, element));
		    assert(pair.second);
		    
		    p = pair.first;
		    element->identity = &p->first;
		    
		    _evictorList.push_front(p);
		    element->position = _evictorList.begin();
		}
		
		//
		// Add all the new facets (recursively)
		//
		EvictorElementPtr& element = p->second;
		
		addFacetImpl(element, servant, FacetPath(), replacing);
		
		//
		// Evict as many elements as necessary.
		//
		evict();
		break; // for(;;)
	    }
	    else
	    {
		loadedElementGeneration = _generation;
	    }
	}

	//
	// Try to load element and try again
	//
	assert(loadedElement == 0);
	assert(triedToLoadElement == false);
	loadedElement = load(ident);
	triedToLoadElement = true;
    }
   
    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "created \"" << ident << "\"";
    }
}

void
Freeze::EvictorI::addFacet(const Identity& ident, const FacetPath& facetPath, const ObjectPtr& servant)
{
    if(facetPath.size() == 0)
    {
	throw EmptyFacetPathException(__FILE__, __LINE__);
    }

    EvictorElementPtr loadedElement = 0;
    int loadedElementGeneration = 0;

    for(;;)
    {
	{
	    Lock sync(*this);
	
	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }
	    
	    EvictorMap::iterator p = _evictorMap.find(ident);
	    
	    if(p == _evictorMap.end() && loadedElement != 0)
	    {
		//
		// If generation matches, load element into map
		//
		if(loadedElementGeneration == _generation)
		{
		    p = insertElement(0, ident, loadedElement);
		}
		else
		{
		    //
		    // Discard loaded element
		    //
		    loadedElement = 0;
		}
	    }

	    if(p != _evictorMap.end())
	    {
		EvictorElementPtr& element = p->second;
		FacetPath parentPath(facetPath);
		parentPath.pop_back();
		FacetMap::iterator q = element->facets.find(parentPath);
		if(q == element->facets.end())
		{
		    throw FacetNotExistException(__FILE__, __LINE__);
		}
		
		{
		    FacetPtr& facet = q->second;
		    IceUtil::Mutex::Lock lockFacet(facet->mutex);
		    
		    if(facet->status == dead || facet->status == destroyed)
		    {
			throw FacetNotExistException(__FILE__, __LINE__);
		    }

		    //
		    // Throws AlreadyRegisteredException if the facet is already registered
		    //
		    facet->rec.servant->ice_addFacet(servant, facetPath[facetPath.size() - 1]);
		}
		//
		// We may need to replace (nested) dead or destroyed facets
		//
		addFacetImpl(element, servant, facetPath, true);

		evict();

		break; // for(;;)
	    }
	   
	    loadedElementGeneration = _generation;
	}

	assert(loadedElement == 0);

	//
	// Load object and loop
	//
	loadedElement = load(ident);
	if(loadedElement == 0)
	{
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
    }

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "added facet to \"" << ident << "\"";
    }
}

void
Freeze::EvictorI::destroyObject(const Identity& ident)
{
    EvictorElementPtr loadedElement = 0;
    int loadedElementGeneration = 0;
    bool triedToLoadElement = false;

    for(;;)
    {
	{
	    Lock sync(*this);
	    
	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }
	    
	    EvictorMap::iterator p = _evictorMap.find(ident);
	   
	    if(p == _evictorMap.end() && triedToLoadElement)
	    {
		if(loadedElementGeneration == _generation)
		{
		    if(loadedElement != 0)
		    {
			p = insertElement(0, ident, loadedElement);
		    }
		}
		else
		{
		    loadedElement = 0;
		    triedToLoadElement = false;
		}
	    }
	    
	    bool destroying = (p != _evictorMap.end());

	    if(destroying || triedToLoadElement)
	    {
		if(destroying)
		{
		    EvictorElementPtr& element = p->second;
		    
		    //
		    // Destroy all existing facets
		    //
		    for(FacetMap::iterator q = element->facets.begin(); q != element->facets.end(); q++)
		    {
			destroyFacetImpl(q, q->second);
		    }
		}

		//
		// Evict as many elements as necessary.
		//
		evict();
		break; // for(;;)
	    }
	    else
	    {
		loadedElementGeneration = _generation;
	    }
	}

	//
	// Try to load element and try again
	//
	assert(loadedElement == 0);
	assert(triedToLoadElement == false);
	loadedElement = load(ident);
	triedToLoadElement = true;
    }

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "destroyed \"" << ident << "\"";
    }
}

ObjectPtr
Freeze::EvictorI::removeFacet(const Identity& ident, const FacetPath& facetPath)
{
    if(facetPath.size() == 0)
    {
	throw EmptyFacetPathException(__FILE__, __LINE__);
    }

    ObjectPtr result = 0;
    EvictorElementPtr loadedElement = 0;
    int loadedElementGeneration = 0;

    for(;;)
    {
	{
	    Lock sync(*this);
	
	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }
	    
	    EvictorMap::iterator p = _evictorMap.find(ident);
	    
	    if(p == _evictorMap.end() && loadedElement != 0)
	    {
		//
		// If generation matches, load element into map
		//
		if(loadedElementGeneration == _generation)
		{
		    p = insertElement(0, ident, loadedElement);
		}
		else
		{
		    //
		    // Discard loaded element
		    //
		    loadedElement = 0;
		}
	    }

	    if(p != _evictorMap.end())
	    {
		EvictorElementPtr& element = p->second;
		FacetPath parentPath(facetPath);
		parentPath.pop_back();
		FacetMap::iterator q = element->facets.find(parentPath);
		if(q == element->facets.end())
		{
		    throw FacetNotExistException(__FILE__, __LINE__);
		}
		
		{
		    FacetPtr& facet = q->second;
		    IceUtil::Mutex::Lock lockFacet(facet->mutex);
		    
		    if(facet->status == dead || facet->status == destroyed)
		    {
			throw FacetNotExistException(__FILE__, __LINE__);
		    }

		    //
		    // Throws NotRegisteredException if the facet is not registered
		    //
		    result = facet->rec.servant->ice_removeFacet(facetPath[facetPath.size() - 1]);
		}
		removeFacetImpl(element->facets, facetPath);

		evict();

		break; // for(;;)
	    }
	   
	    loadedElementGeneration = _generation;
	}

	assert(loadedElement == 0);

	//
	// Load object and loop
	//
	loadedElement = load(ident);
	if(loadedElement == 0)
	{
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
    }

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Freeze.Evictor");
	out << "removed facet from \"" << ident << "\"";
    }
    return result;
}


void
Freeze::EvictorI::removeAllFacets(const Identity& ident)
{
    EvictorElementPtr loadedElement = 0;
    int loadedElementGeneration = 0;

    for(;;)
    {
	{
	    Lock sync(*this);
	
	    if(_deactivated)
	    {
		throw EvictorDeactivatedException(__FILE__, __LINE__);
	    }
	    
	    EvictorMap::iterator p = _evictorMap.find(ident);
	    
	    if(p == _evictorMap.end() && loadedElement != 0)
	    {
		//
		// If generation matches, load element into map
		//
		if(loadedElementGeneration == _generation)
		{
		    p = insertElement(0, ident, loadedElement);
		}
		else
		{
		    //
		    // Discard loaded element
		    //
		    loadedElement = 0;
		}
	    }

	    if(p != _evictorMap.end())
	    {
		EvictorElementPtr& element = p->second;
		
		{
		    FacetPtr& facet = element->mainObject;
		    IceUtil::Mutex::Lock lockFacet(facet->mutex);
		    
		    if(facet->status == dead || facet->status == destroyed)
		    {
			throw ObjectNotExistException(__FILE__, __LINE__);
		    }
		    facet->rec.servant->ice_removeAllFacets();
		}
		
		{
		    //
		    // Destroy all facets except main object
		    //
		    for(FacetMap::iterator q = element->facets.begin(); q != element->facets.end(); q++)
		    {
			if(q->second != element->mainObject)
			{
			    destroyFacetImpl(q, q->second);
			}
		    }
		}

		evict();

		break; // for(;;)
	    }
	   
	    loadedElementGeneration = _generation;
	}

	assert(loadedElement == 0);

	//
	// Load object and loop
	//
	loadedElement = load(ident);
	if(loadedElement == 0)
	{
	    throw ObjectNotExistException(__FILE__, __LINE__);
	}
    }

    if(_trace >= 1)
    {
	Trace out(_communicator->getLogger(), "Ereeze.Evictor");
	out << "removed all facets from \"" << ident << "\"";
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
Freeze::EvictorI::getIterator(Int batchSize, bool loadServants)
{
    {
	Lock sync(*this);
	if(_deactivated)
	{
	    throw EvictorDeactivatedException(__FILE__, __LINE__);
	}
	saveNowNoSync();
    }
    
    return new EvictorIteratorI(*this, batchSize, loadServants);
}

bool
Freeze::EvictorI::hasObject(const Identity& ident)
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
	IceUtil::Mutex::Lock lockFacet(element->mainObject->mutex);
	return (element->mainObject->status != destroyed && element->mainObject->status != dead);
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
    EvictorElementPtr loadedElement = 0;
    int loadedElementGeneration = 0;
    cookie = 0;
   
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
	   
	    if(p == _evictorMap.end() && loadedElement != 0)
	    {
		if(loadedElementGeneration == _generation)
		{
		    p = insertElement(current.adapter, current.id, loadedElement);
		}
		else
		{
		    loadedElement = 0;
		}
	    }
	    
	    objectFound = (p != _evictorMap.end());

	    if(objectFound)
	    {
		//
		// Ice object found in evictor map. Push it to the front of
		// the evictor list, so that it will be evicted last.
		//
		EvictorElementPtr& element = p->second;
		if(element->position != _evictorList.begin())
		{
		    _evictorList.erase(element->position);
		    _evictorList.push_front(p);
		    element->position = _evictorList.begin();
		}
       
		element->usageCount++;

		FacetMap::iterator q = element->facets.find(current.facet);
		if(q != element->facets.end())
		{
		    cookie = q->second;
		}
	
		evict();
		//
		// Later (after releasing the mutex), check that this
		// object is not dead or destroyed
		//
	    }
	    else
	    {
		loadedElementGeneration = _generation;
	    }
	}
	
	if(objectFound)
	{
	    EvictorElementPtr& element = p->second;

	    if(_trace >= 2)
	    {
		Trace out(_communicator->getLogger(), "Freeze.Evictor");
		out << "found \"" << current.id << "\" in the queue";
	    }
	    
	    //
	    // Return servant if object not dead or destroyed
	    //
	    if(cookie == 0)
	    {
		ObjectPtr result = 0;
		{
		    IceUtil::Mutex::Lock lockFacet(element->mainObject->mutex);
		    if(element->mainObject->status != destroyed && element->mainObject->status != dead)
		    {
			result = element->mainObject->rec.servant;
		    }
		}
		if(_trace >= 2)
		{
		    Trace out(_communicator->getLogger(), "Freeze.Evictor");
		    out << "\"" << current.id << "\" does not have the desired facet";
		}
		Lock sync(*this);
		element->usageCount--;
		return result;
	    }
	    else
	    {
		IceUtil::Mutex::Lock lockFacet(element->mainObject->mutex);
		if(element->mainObject->status != destroyed && element->mainObject->status != dead)
		{
		    return element->mainObject->rec.servant;
		}
	    }
	
	    //
	    // Object is destroyed or dead: clean-up
	    //
	    if(_trace >= 2)
	    {
		Trace out(_communicator->getLogger(), "Freeze.Evictor");
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
		Trace out(_communicator->getLogger(), "Freeze.Evictor");
		out << "could not find \"" << current.id << "\" in the queue\n"
		    << "loading \"" << current.id << "\" from the database";
	    }
	    
	    loadedElement = load(current.id);
	    if(loadedElement == 0)
	    {
		return 0;
	    }
	}
    }
}

void
Freeze::EvictorI::finished(const Current& current, const ObjectPtr& servant, const LocalObjectPtr& cookie)
{
    assert(servant);

    if(cookie != 0)
    {
	FacetPtr facet = FacetPtr::dynamicCast(cookie);
	assert(facet);
    
	bool enqueue = false;
	
	if(current.mode != Nonmutating)
	{
	    IceUtil::Mutex::Lock lockRec(facet->mutex);
	    
	    if(facet->status == clean)
	    {
		//
		// Assume this operation updated the object
		// 
		facet->status = modified;
		enqueue = true;
	    }
	}
	
	Lock sync(*this);
	
	assert(!_deactivated);
	
	//
	// Decrease the usage count of the evictor queue element.
	//
	assert(facet->element->usageCount >= 1);
	--facet->element->usageCount;
	
	if(enqueue)
	{
	    FacetMap::iterator q = facet->element->facets.find(current.facet);
	    assert(q != facet->element->facets.end());
	    
	    addToModifiedQueue(q, facet);
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
	
    if(!_deactivated)
    {
	saveNowNoSync();
       	
	if(_trace >= 1)
	{
	    Trace out(_communicator->getLogger(), "Freeze.Evictor");
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
	catch(const DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	_db.reset();
	_dbEnv = 0;
	_dbEnvHolder = 0;
	_initializer = 0;
    }
}

void
Freeze::EvictorI::run()
{
    for(;;)
    {
    	deque<FacetMap::iterator> allObjects;
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
	
	Long saveStart = IceUtil::Time::now().toMilliSeconds();
	
	//
	// Stream each element
	//
	for(size_t i = 0; i < size; i++)
	{
	    FacetPtr& facet = allObjects[i]->second;
	    
	    bool tryAgain;
	    do
	    {
		tryAgain = false;
		ObjectPtr servant = 0;

		IceUtil::Mutex::Lock lockFacet(facet->mutex);
		Byte status = facet->status;
		
		switch(status)
		{
		    case created:
		    case modified:
		    {
			servant = facet->rec.servant;
			break;
		    }   
		    case destroyed:
		    {
			size_t index = streamedObjectQueue.size();
			streamedObjectQueue.resize(index + 1);
			StreamedObject& obj = streamedObjectQueue[index];
			streamFacet(facet, allObjects[i]->first, status, saveStart, obj);
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
		    lockFacet.release();
		}
		else
		{
		    IceUtil::AbstractMutex* mutex = dynamic_cast<IceUtil::AbstractMutex*>(servant.get());
		    if(mutex != 0)
		    {
			//
			// Lock servant and then facet so that user can safely lock
			// servant and call various Evictor operations
			//

			IceUtil::AbstractMutex::TryLock lockServant(*mutex);
			if(!lockServant.acquired())
			{
			    lockFacet.release();
			    lockServant.acquire();
			    lockFacet.acquire();
			    status = facet->status;
			}
			
			switch(status)
			{
			    case created:
			    case modified:
			    {
				if(servant == facet->rec.servant)
				{
				    facet->status = clean;
				    size_t index = streamedObjectQueue.size();
				    streamedObjectQueue.resize(index + 1);
				    StreamedObject& obj = streamedObjectQueue[index];
				    streamFacet(facet, allObjects[i]->first, status, saveStart, obj);
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
				facet->status = dead;
				size_t index = streamedObjectQueue.size();
				streamedObjectQueue.resize(index + 1);
				StreamedObject& obj = streamedObjectQueue[index];
				streamFacet(facet, allObjects[i]->first, status, saveStart, obj);
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
			if(_noSyncAllowed)
			{
			    facet->status = clean;
			    size_t index = streamedObjectQueue.size();
			    streamedObjectQueue.resize(index + 1);
			    StreamedObject& obj = streamedObjectQueue[index];
			    streamFacet(facet, allObjects[i]->first, status, saveStart, obj);
			}
			else
			{
			    DatabaseException ex(__FILE__, __LINE__);
			    ex.message = string(typeid(*facet->rec.servant).name()) 
				+ " does not implement IceUtil::AbstractMutex and Freeze.Evictor.NoSyncAllowed is 0";
			    throw ex;
			}
		    }
		}
	    } while(tryAgain);
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
		
		try
		{
		    DbTxn* tx = 0;
		    _dbEnv->txn_begin(0, &tx, 0);
		    try
		    {   
			for(size_t i = 0; i < txSize; i++)
			{
			    StreamedObject& obj = streamedObjectQueue[i];

			    switch(obj.status)
			    {
				case created:
				case modified:
				{
				    Dbt dbKey;
				    Dbt dbValue;
				    initializeInDbt(obj.key, dbKey);
				    initializeInDbt(obj.value, dbValue);
				    u_int32_t flags = (obj.status == created) ? DB_NOOVERWRITE : 0;
				    int err = _db->put(tx, &dbKey, &dbValue, flags);
				    if(err != 0)
				    {
					throw DatabaseException(__FILE__, __LINE__);
				    }
				    break;
				}
				case destroyed:
				{
				    Dbt dbKey;
				    initializeInDbt(obj.key, dbKey);
				    int err = _db->del(tx, &dbKey, 0);
				    if(err != 0)
				    {
					throw DatabaseException(__FILE__, __LINE__);
				    }
				    break;
				}   
				default:
				{
				    assert(0);
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
		    streamedObjectQueue.erase
			(streamedObjectQueue.begin(), 
			 streamedObjectQueue.begin() + txSize);
		    
		    if(_trace >= 1)
		    {
			Long now = IceUtil::Time::now().toMilliSeconds();
			Trace out(_communicator->getLogger(), "Freeze.Evictor");
			out << "saved " << txSize << " objects in " 
			    << static_cast<Int>(now - saveStart) << " ms";
			saveStart = now;
		    }
		}
		catch(const DbDeadlockException&)
		{
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
	    
	    _generation++;

	    for(deque<FacetMap::iterator>::iterator q = allObjects.begin();
		q != allObjects.end(); q++)
	    {
		(*q)->second->element->usageCount--;
	    }
	    allObjects.clear();
	    evict();
	    
	    if(saveNowThreadsSize > 0)
	    {
		_saveNowThreads.erase(_saveNowThreads.begin(), _saveNowThreads.begin() + saveNowThreadsSize);
		notifyAll();
	    }
	}
	_lastSave = IceUtil::Time::now();
    }
}


bool
Freeze::EvictorI::load(const Identity& ident, Dbc* dbc, 
		       Key& key, Value& value, vector<EvictorElementPtr>& evictorElements)
{
    Key root;
    marshalRoot(ident, root, _communicator);

    Dbt dbKey;
    Dbt dbValue;

    EvictorElementPtr elt = new EvictorElement;
    int rs = 0;
    do
    {
	//
	// Unmarshal key and data and insert it into elt's facet map
	//
	EvictorStorageKey esk;
	unmarshal(esk, key, _communicator);
       
	if(_trace >= 3)
	{
	    Trace out(_communicator->getLogger(), "Freeze.Evictor");
	    out << "reading facet identity = \"" << esk.identity << "\" ";
	    if(esk.facet.size() == 0)
	    {
		out << "(main object)";
	    }
	    else
	    {
		out << "facet = \"";
		for(size_t i = 0; i < esk.facet.size(); i++)
		{
		    out << esk.facet[i];
		    if(i != esk.facet.size() - 1)
		    {
			out << ".";
		    }
		    else
		    {
			out << "\"";
		    }
		}
	    }
	}

	FacetPtr facet = new Facet(elt.get());
	facet->status = clean;
	unmarshal(facet->rec, value, _communicator);
	
	pair<FacetMap::iterator, bool> pair;
	pair = elt->facets.insert(FacetMap::value_type(esk.facet, facet));
	assert(pair.second);
	
	if(esk.facet.size() == 0)
	{
	    elt->mainObject = facet;
	}
	
	initializeOutDbt(key, dbKey);
	initializeOutDbt(value, dbValue);

	for(;;)
	{
	    try
	    {
		rs = dbc->get(&dbKey, &dbValue, DB_NEXT);
		if(rs == 0)
		{
		    //
		    // Key may be used as input of a DB_SET_RANGE call, so we
		    // need to write its exact size
		    //
		    key.resize(dbKey.get_size());
		}
		break; // for(;;)
	    }
	    catch(const DbMemoryException& dx)
	    {
		handleMemoryException(dx, key, dbKey, value, dbValue);
	    }   
	}
    }
    while(rs == 0 && startWith(key, root));
    
    buildFacetMap(elt->facets);	
    evictorElements.push_back(elt);
    return (rs == 0);
}

bool
Freeze::EvictorI::skipFacets(const Identity& ident, Dbc* dbc, Key& key)
{
    Key root;
    marshalRoot(ident, root, _communicator);

    Dbt dbKey;
    Dbt dbValue;
  
    int rs = 0;
    do
    {
	initializeOutDbt(key, dbKey);
	dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);

	for(;;)
	{
	    try
	    {
		rs = dbc->get(&dbKey, &dbValue, DB_NEXT);
		if(rs == 0)
		{
		    key.resize(dbKey.get_size());
		}
		break; // for (;;)
	    }
	    catch(const DbMemoryException& dx)
	    {
		handleMemoryException(dx, key, dbKey);
	    }
	}
    }
    while(rs == 0 && startWith(key, root));
    return (rs == 0);
}

void
Freeze::EvictorI::insert(const vector<Identity>& identities, 
			 const vector<EvictorElementPtr>& evictorElements,
			 int loadedGeneration)
{
    assert(identities.size() == evictorElements.size());
	
    size_t size = identities.size();
	
    if(size > 0)
    {
	Lock sync(*this);

	if(_deactivated)
	{
	    throw EvictorDeactivatedException(__FILE__, __LINE__);
	}
	
	if(_generation == loadedGeneration)
	{
	    for(size_t i = 0; i < size; ++i)
	    {
		const Identity& ident = identities[i];
		
		EvictorMap::iterator p = _evictorMap.find(ident);
		if(p == _evictorMap.end())
		{
		    p = insertElement(0, ident, evictorElements[i]);
		}
	    }
	}
	//
	// Otherwise we don't insert anything
	//
	    
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
	    Trace out(_communicator->getLogger(), "Freeze.Evictor");
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
Freeze::EvictorI::dbHasObject(const Identity& ident)
{
    EvictorStorageKey esk;
    esk.identity = ident;
    
    Key key;    
    marshal(esk, key, _communicator);
    Dbt dbKey;
    initializeInDbt(key, dbKey);
    
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
		throw DatabaseException(__FILE__, __LINE__);
	    }
	}
	catch(const DbDeadlockException&)
	{
	    //
	    // Ignored, try again
	    //
	}
	catch(const DbException& dx)
	{
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
    }
}

void
Freeze::EvictorI::addToModifiedQueue(const Freeze::EvictorI::FacetMap::iterator& q,
				     const Freeze::EvictorI::FacetPtr& facet)
{
    facet->element->usageCount++;
    _modifiedQueue.push_back(q);
    
    if(_saveSizeTrigger >= 0 && static_cast<Int>(_modifiedQueue.size()) >= _saveSizeTrigger)
    {
	notifyAll();
    }
}

void
Freeze::EvictorI::streamFacet(const FacetPtr& facet, const FacetPath& facetPath, Byte status, 
			      Long saveStart, StreamedObject& obj)
{
    EvictorStorageKey esk;
    esk.identity.name = facet->element->identity->name;
    esk.identity.category = facet->element->identity->category;
    esk.facet = facetPath;
    marshal(esk, obj.key, _communicator);
    obj.status = status;
    if(status != destroyed)
    {
	writeObjectRecordToValue(saveStart, facet->rec, obj.value);
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
    }
    while(find(_saveNowThreads.begin(), _saveNowThreads.end(), myself) != _saveNowThreads.end());
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
    
    marshal(rec, value, _communicator);
}

Freeze::EvictorI::EvictorElementPtr
Freeze::EvictorI::load(const Identity& ident)
{
    //
    // This method attempts to restore an object and all of its facets from the database. It works by
    // iterating over the database keys that match the "root" key. The root key is the encoded portion
    // of the EvictorStorageKey struct that the object and its facets all have in common, namely the
    // identity.
    //
    Key root;
    marshalRoot(ident, root, _communicator);

    Key key(root);
    const size_t defaultKeySize = 1024;
    key.resize(defaultKeySize);

    const size_t defaultValueSize = 1024;
    Value value(defaultValueSize);

    EvictorElementPtr result;

    for(;;)
    {
	result = new EvictorElement;

	Dbc* dbc = 0;
	int rs = 0;

	try
	{
	    //
	    // Open cursor
	    //
	    _db->cursor(0, &dbc, 0);

            //
            // We position the cursor at the key for the main object.
            //
	    Dbt dbKey;
	    initializeOutDbt(key, dbKey);
	    dbKey.set_size(root.size());

	    Dbt dbValue;
	    initializeOutDbt(value, dbValue);

	    //
	    // Get first pair
	    //
	    for(;;)
	    {
		try
		{
		    rs = dbc->get(&dbKey, &dbValue, DB_SET_RANGE);
		    break;
		}
		catch(const DbMemoryException& dx)
		{
		    handleMemoryException(dx, key, dbKey, value, dbValue);
		}
	    }

	    while(rs == 0 && startWith(key, root))
	    {
		//
		// Unmarshal key and data and insert it into result's facet map
		//
		EvictorStorageKey esk;
		unmarshal(esk, key, _communicator);
	
		if(_trace >= 3)
		{
		    Trace out(_communicator->getLogger(), "Freeze.Evictor");
		    out << "reading facet identity = \"" << esk.identity << "\" ";
		    if(esk.facet.size() == 0)
		    {
			out << "(main object)";
		    }
		    else
		    {
			out << "facet = \"";
			for(size_t i = 0; i < esk.facet.size(); i++)
			{
			    out << esk.facet[i];
			    if(i != esk.facet.size() - 1)
			    {
				out << ".";
			    }
			    else
			    {
				out << "\"";
			    }
			}
		    }
		}
       
		//
		// The Ice encoding of Identity is such that startWith(key, root)
		// implies esk.identity == ident
		//
		assert(esk.identity == ident);
	
		FacetPtr facet = new Facet(result.get());
		facet->status = clean;
		unmarshal(facet->rec, value, _communicator);
		
		pair<FacetMap::iterator, bool> pair;
		pair = result->facets.insert(FacetMap::value_type(esk.facet, facet));
		assert(pair.second);
		
		if(esk.facet.size() == 0)
		{
		    result->mainObject = facet;
		}
		
		//
		// Next facet
		//
		initializeOutDbt(key, dbKey);
		initializeOutDbt(value, dbValue);
						    
		for(;;)
		{
		    try
		    {
			rs = dbc->get(&dbKey, &dbValue, DB_NEXT);
			break; // for(;;)
		    }
		    catch(const DbMemoryException& dx)
		    {
			handleMemoryException(dx, key, dbKey, value, dbValue);
		    }
		}
	    }

	    Dbc* toClose = dbc;
	    dbc = 0;
	    toClose->close();
	    break; // for (;;)
	}
	catch(const DbDeadlockException&)
	{
	    if(dbc != 0)
	    {
		try
		{
		    dbc->close();
		}
		catch(...)
		{
		}
	    }
	    
	    //
	    // Try again
	    //
	}
	catch(const DbException& dx)
	{
	    if(dbc != 0)
	    {
		try
		{
		    dbc->close();
		}
		catch(...)
		{
		}
	    }
	    DatabaseException ex(__FILE__, __LINE__);
	    ex.message = dx.what();
	    throw ex;
	}
	catch(...)
	{
	    try
	    {
		dbc->close();
	    }
	    catch(...)
	    {
	    }
	    throw;
	}
    }

    if(result->facets.size() == 0)
    {
	if(_trace >= 2)
	{
	    Trace out(_communicator->getLogger(), "Freeze.Evictor");
	    out << "could not find \"" << ident << "\" in the database";
	}
	return 0;
    }

    buildFacetMap(result->facets);
    return result;
}

Freeze::EvictorI::EvictorMap::iterator
Freeze::EvictorI::insertElement(const ObjectAdapterPtr& adapter, const Identity& ident, const EvictorElementPtr& element)
{
    if(_initializer)
    {
	_initializer->initialize(adapter, ident, element->mainObject->rec.servant);
    }

    pair<EvictorMap::iterator, bool> pair = _evictorMap.insert(EvictorMap::value_type(ident, element));
    assert(pair.second);
    EvictorMap::iterator p = pair.first;
    element->identity = &p->first;

    _evictorList.push_front(p);
    element->position = _evictorList.begin();

    return p;
}


void
Freeze::EvictorI::addFacetImpl(EvictorElementPtr& element, const ObjectPtr& servant, 
			       const FacetPath& facetPath, bool replacing)
{
    FacetMap& facets = element->facets;

    bool insertIt = true;

    if(replacing)
    {
	FacetMap::iterator q = facets.find(facetPath);
	
	if(q != facets.end())
	{
	    FacetPtr& facet = q->second;

	    {
		IceUtil::Mutex::Lock lockFacet(facet->mutex);
		
		switch(facet->status)
		{
		    case clean:
		    {
			facet->status = modified;
			addToModifiedQueue(q, facet);
			break;
		    }
		    case created:
		    case modified:
		    {
			//
			// Nothing to do.
			// No need to push it on the modified queue as a created resp
			// modified facet is either already on the queue or about 
			// to be saved. When saved, it becomes clean.
			//
			break;
		    }  
		    case destroyed:
		    {
			facet->status = modified;
			//
			// No need to push it on the modified queue, as a destroyed facet
			// is either already on the queue or about to be saved. When saved,
			// it becomes dead.
			//
			break;
		    }
		    case dead:
		    {
			facet->status = created;
			addToModifiedQueue(q, facet);
			break;
		    }
		    default:
		    {
			assert(0);
			break;
		    }
		}
		facet->rec.servant = servant;
		insertIt = false;
	    }
	}
    }

    if(insertIt)
    {
	FacetPtr facet = new Facet(element.get());
	facet->status = created;

	ObjectRecord& rec = facet->rec;
	rec.servant = servant;
	rec.stats.creationTime = IceUtil::Time::now().toMilliSeconds();
	rec.stats.lastSaveTime = 0;
	rec.stats.avgSaveTime = 0;

	pair<FacetMap::iterator, bool> insertResult = facets.insert(FacetMap::value_type(facetPath, facet));
	assert(insertResult.second);
	if(facetPath.size() == 0)
	{
	    element->mainObject = facet;
	}
	addToModifiedQueue(insertResult.first, facet);
    }

    
    if(servant != 0)
    {
	//
	// Add servant's facets
	//
	vector<string> facetList = servant->ice_facets();
	for(vector<string>::iterator r = facetList.begin(); r != facetList.end(); r++)
	{
	    FacetPath newFacetPath(facetPath);
	    newFacetPath.push_back(*r);
	    addFacetImpl(element, servant->ice_findFacet(*r), newFacetPath, replacing);  
	}
    }
}


void
Freeze::EvictorI::removeFacetImpl(FacetMap& facets, const FacetPath& facetPath)
{
    FacetMap::iterator q = facets.find(facetPath);
    ObjectPtr servant = 0; 

    if(q != facets.end())
    {
	servant = destroyFacetImpl(q, q->second);
    }
    //
    // else should we raise an exception?
    //

    if(servant != 0)
    {
	//
	// Remove servant's facets
	//
	vector<string> facetList = servant->ice_facets();
	for(vector<string>::iterator r = facetList.begin(); r != facetList.end(); r++)
	{
	    FacetPath newFacetPath(facetPath);
	    newFacetPath.push_back(*r);
	    removeFacetImpl(facets, newFacetPath);  
	}
    }
}


ObjectPtr
Freeze::EvictorI::destroyFacetImpl(Freeze::EvictorI::FacetMap::iterator& q, const Freeze::EvictorI::FacetPtr& facet)
{
    IceUtil::Mutex::Lock lockFacet(facet->mutex);
    switch(facet->status)
    {
	case clean:
	{
	    facet->status = destroyed;
	    addToModifiedQueue(q, facet);
	    break;
	}
	case created:
	{
	    facet->status = dead;
	    break;
	}
	case modified:
	{
	    facet->status = destroyed;
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
    return facet->rec.servant;
}

void
Freeze::EvictorI::buildFacetMap(const FacetMap& facets)
{
    for(FacetMap::const_iterator q = facets.begin(); q != facets.end(); q++)
    {
	const FacetPath& facetPath = q->first;

	if(facetPath.size() > 0)
	{
	    FacetPath parent(facetPath);
	    parent.pop_back();
	    FacetMap::const_iterator r = facets.find(parent);
	    if(r == facets.end())
	    {
		// 
		// TODO: log warning for this orphan facet
		//
		assert(0);
	    }
	    else
	    {
		r->second->rec.servant->ice_addFacet(q->second->rec.servant, facetPath[facetPath.size() - 1]); 
	    }
	}
    }


}


Freeze::EvictorI::Facet::Facet(EvictorElement* elt) :
    status(dead),
    element(elt)
{
}

Freeze::EvictorI::EvictorElement::EvictorElement() :
    usageCount(0),
    identity(0),
    mainObject(0)
{
}

Freeze::EvictorI::EvictorElement::~EvictorElement()
{
}

Freeze::EvictorIteratorI::EvictorIteratorI(EvictorI& evictor, Int batchSize, bool loadServants) :
    _evictor(evictor),
    _batchSize(batchSize),
    _loadServants(loadServants),
    _key(1024),
    _more(true)
{
    if(loadServants)
    {
	_value.resize(1024);
    }
    _batchIterator = _batch.end();
}


bool
Freeze::EvictorIteratorI::hasNext()
{
    if(_batchIterator != _batch.end()) 
    {
	return true;
    }
    else
    {
	_batchIterator = nextBatch();
	return (_batchIterator != _batch.end());
    }
}

Identity
Freeze::EvictorIteratorI::next()
{
    if(hasNext())
    {
	return *_batchIterator++;
    }
    else
    {
	throw Freeze::NoSuchElementException(__FILE__, __LINE__);
    }
}


vector<Identity>::const_iterator
Freeze::EvictorIteratorI::nextBatch()
{
    _batch.clear();

    if(!_more)
    {
	return _batch.end();
    }

    vector<EvictorI::EvictorElementPtr> evictorElements;
    evictorElements.reserve(_batchSize);
     
    Key previousKey = _key;
    int loadedGeneration = 0;

    try
    {
	for(;;)
	{
	    _batch.clear();
	    evictorElements.clear();
	    
	    Int count = _batchSize;
	    
	    Dbt dbKey;
	    initializeOutDbt(_key, dbKey);

	    Dbt dbValue;
	    if(_loadServants)
	    {
		initializeOutDbt(_value, dbValue);
	    }
	    else
	    {
		dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
	    }

	    Dbc* dbc = 0;
	    try
	    {
		//
		// Move to the first record
		// 
		uint32_t flags = DB_NEXT;
		if(_key.size() > 0)
		{
		    //
		    // _key represents the next element not yet returned
		    // if it has been deleted, we want the one after
		    //
		    flags = DB_SET_RANGE;

		    //
		    // Will be used as input as well
		    //
		    dbKey.set_size(previousKey.size());
		}
		
		if(_loadServants)
		{
		    loadedGeneration = _evictor.currentGeneration();
		}

		_evictor.db()->cursor(0, &dbc, 0);
		
		for(;;)
		{
		    try
		    {
			_more = (dbc->get(&dbKey, &dbValue, flags) == 0);
			if(_more)
			{
			    _key.resize(dbKey.get_size());
			    //
			    // No need to resize data as we never use it as input
			    //
			}
			break;
		    }
		    catch(const DbMemoryException& dx)
		    {
			handleMemoryException(dx, _key, dbKey, _value, dbValue);
		    }
		}
		
		while(count > 0 && _more)
		{
		    EvictorStorageKey esk;
		    unmarshal(esk, _key, _evictor.communicator());
		    
		    //
		    // Because of the Ice encoding and default binary comparison, records with
		    // facet length = 0 are before records with facet length > 0 (for a given
		    // identity).
		    //
		    assert(esk.facet.size() == 0);
		    
		    const Identity& ident = esk.identity;
		    _batch.push_back(ident);
		    count--;
		    
		    //
		    // Even when count is 0, we read one more record (unless we reach the end)
		    //
		    if(_loadServants)
		    {
			_more = _evictor.load(ident, dbc, _key, _value, evictorElements);
		    }
		    else
		    {
			_more = _evictor.skipFacets(ident, dbc, _key);
		    }
		}
		
		Dbc* toClose = dbc;
		dbc = 0;
		toClose->close();
		break; // for (;;)
	    }
	    catch(const DbDeadlockException&)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}
		_key = previousKey;
		//
		// Retry
		//
	    }
	    catch(...)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}
		throw;
	    }
	}
    }
    catch(const DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    
    if(_batch.size() == 0)
    {
	return _batch.end();
    }
    else
    {
	if(_loadServants)
	{
	    _evictor.insert(_batch, evictorElements, loadedGeneration);
	}
	return _batch.begin();
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
Freeze::EmptyFacetPathException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nempty facet path";
}
