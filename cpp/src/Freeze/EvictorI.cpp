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
#include <Freeze/Initialize.h>
#include <sys/stat.h>
#include <IceUtil/AbstractMutex.h>
#include <IceXML/StreamI.h>
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
    Freeze::EvictorStorageKey _current;
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

inline bool startWith(Key key, Key root)
{
    if(root.size() > key.size())
    {
	return false;
    }
    return memcmp(&root[0], &key[0], root.size()) == 0;
}


//
// Marshaling/unmarshaling persistent (key, data) pairs
//
// TODO: use template functions
//

void marshalRoot(const EvictorStorageKey& v, Key& bytes, const CommunicatorPtr& communicator)
{
    ostringstream ostr;
    StreamPtr stream = new IceXML::StreamI(communicator, ostr);
    v.ice_marshal("Key", stream);
    const string& str = ostr.str();

    //
    // TODO: fix this!
    //
    int index = str.find("</identity>");
    string root = str.substr(0, index + strlen("</identity>"));
    
    bytes.resize(root.size());
    std::copy(root.begin(), root.end(), bytes.begin());
}

void marshal(const EvictorStorageKey& v, Key& bytes, const CommunicatorPtr& communicator)
{
    ostringstream ostr;
    StreamPtr stream = new IceXML::StreamI(communicator, ostr);
    v.ice_marshal("Key", stream);
    const string& str = ostr.str();
    // cerr << "Marshalled key == " << str << endl;
    bytes.resize(str.size());
    std::copy(str.begin(), str.end(), bytes.begin());
}

void unmarshal(EvictorStorageKey& v, const Key& bytes, const CommunicatorPtr& communicator)
{
    string str;
    str.append("<data>");
    str.append(reinterpret_cast<const char*>(&bytes[0]), bytes.size());
    str.append("</data>");
    // cerr << "esk to unmarshal == " << str << endl;
    istringstream istr(str);
    StreamPtr stream = new IceXML::StreamI(communicator, istr, false);
    v.ice_unmarshal("Key", stream);
}

void marshal(const ObjectRecord& v, Value& bytes, const CommunicatorPtr& communicator)
{
    std::ostringstream ostr;
    StreamPtr stream = new IceXML::StreamI(communicator, ostr);
    stream->marshalFacets(false);
    v.ice_marshal("Value", stream);
    const string& str = ostr.str();

    // cerr << "Marshalled object record == " << str << endl;

    bytes.resize(str.size());
    std::copy(str.begin(), str.end(), bytes.begin());
}

void unmarshal(ObjectRecord& v, const Value& bytes, const CommunicatorPtr& communicator)
{
    string str;
    str.append("<data>");
    str.append(reinterpret_cast<const char*>(&bytes[0]), bytes.size());
    str.append("</data>");
    // cerr << "object record to unmarshal == " << str << endl;
    std::istringstream istr(str);
   
    StreamPtr stream = new IceXML::StreamI(communicator, istr, false);
    v.ice_unmarshal("Value", stream);
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
    _noSyncAllowed(false),
    _generation(0)
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
    _noSyncAllowed(false),
    _generation(0)
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
	Trace out(_communicator->getLogger(), "Evictor");
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
	Trace out(_communicator->getLogger(), "Evictor");
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
	Trace out(_communicator->getLogger(), "Evictor");
	out << "destroyed \"" << ident << "\"";
    }
}

void
Freeze::EvictorI::removeFacet(const Identity& ident, const FacetPath& facetPath)
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
		    // Throws NotRegisteredException if the facet is not registered
		    //
		    facet->rec.servant->ice_removeFacet(facetPath[facetPath.size() - 1]);
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
	Trace out(_communicator->getLogger(), "Evictor");
	out << "removed facet from \"" << ident << "\"";
    }
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
	Trace out(_communicator->getLogger(), "Evictor");
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
		Trace out(_communicator->getLogger(), "Evictor");
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
		    Trace out(_communicator->getLogger(), "Evictor");
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
	    
	    IceUtil::Mutex::Lock lockFacet(facet->mutex);
	    ObjectRecord& rec = facet->rec;
	    
	    bool streamIt = true;
	    Ice::Byte status = facet->status;
	    switch(status)
	    {
		case created:
		case modified:
		{
		    facet->status = clean;
		    break;
		}   
		case destroyed:
		{
		    facet->status = dead;
		    break;
		}   
		default:
		{
		    //
		    // Nothing to do (could be a duplicate)
		    //
		    streamIt = false;
		    break;
		}
	    }
	    
	    if(streamIt)
	    {
		size_t index = streamedObjectQueue.size();
		streamedObjectQueue.resize(index + 1);
		StreamedObject& obj = streamedObjectQueue[index];
		EvictorStorageKey esk;
		esk.identity.name = facet->element->identity->name;
		esk.identity.category = facet->element->identity->category;
		esk.facet = allObjects[i]->first;
		marshal(esk, obj.key, _communicator);
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

			    switch(obj.status)
			    {
				case created:
				case modified:
				{
				    Dbt dbKey;
				    Dbt dbValue;
				    initializeDbt(obj.key, dbKey);
				    initializeDbt(obj.value, dbValue);
				    u_int32_t flags = (obj.status == created) ? DB_NOOVERWRITE : 0;
				    int err = _db->put(tx, &dbKey, &dbValue, flags);
				    if(err != 0)
				    {
					throw DBException(__FILE__, __LINE__);
				    }
				    break;
				}
				case destroyed:
				{
				    Dbt dbKey;
				    initializeDbt(obj.key, dbKey);
				    int err = _db->del(tx, &dbKey, 0);
				    if(err != 0)
				    {
					throw DBException(__FILE__, __LINE__);
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
    EvictorStorageKey esk;
    esk.identity = ident;
    
    Key key;    
    marshal(esk, key, _communicator);
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
    
    marshal(rec, value, _communicator);
}


Freeze::EvictorI::EvictorElementPtr
Freeze::EvictorI::load(const Identity& ident)
{
    Key root;
    EvictorStorageKey esk;
    esk.identity = ident;
    marshalRoot(esk, root, _communicator);
    
    const size_t defaultKeySize = 1024;
    Key key(root);
    key.reserve(defaultKeySize);

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

	    key.resize(key.capacity());
	    Dbt dbKey;
	    initializeDbt(key, dbKey);
	    
	    value.resize(value.capacity());
	    Dbt dbValue;
	    initializeDbt(value, dbValue);

	    //
	    // Get first pair
	    //
	    for(;;)
	    {
		try
		{
		    rs = dbc->get(&dbKey, &dbValue, DB_SET_RANGE);

		    if(rs == 0)
		    {
			key.resize(dbKey.get_size());
			value.resize(dbValue.get_size());
		    }
		    
		    if(rs != 0 || !startWith(key, root))
		    {
			dbc->close();

			if(_trace >= 2)
			{
			    Trace out(_communicator->getLogger(), "Evictor");
			    out << "could not find \"" << ident << "\" in the database";
			}
			return 0;
		    }
		    
		    break;
		}
		catch(const ::DbMemoryException dx)
		{
		    bool resized = false;
		    if(dbKey.get_size() > dbKey.get_ulen())
		    {
			key.resize(dbKey.get_size());
			initializeDbt(key, dbKey);
			resized = true;
		    }
		   
		    if(dbValue.get_size() > dbValue.get_ulen())
		    {
			value.resize(dbValue.get_size());
			initializeDbt(value, dbValue);
			resized = true;
		    }

		    if(!resized)
		    {
			//
			// Real problem
			//
			DBException ex(__FILE__, __LINE__);
			ex.message = dx.what();
			throw ex;
		    }
		}
	    }

	    do
	    {
		//
		// Unmarshal key and data and insert it into result's facet map
		//
		EvictorStorageKey esk;
		unmarshal(esk, key, _communicator);

	
		if(_trace >= 3)
		{
		    Trace out(_communicator->getLogger(), "Evictor");
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
       
		FacetPtr facet = new Facet(result.get());
		facet->status = clean;
		unmarshal(facet->rec, value, _communicator);
	
		pair<FacetMap::iterator, bool> pair = result->facets.insert(FacetMap::value_type(esk.facet, facet));
		assert(pair.second);

		if(esk.facet.size() == 0)
		{
		    result->mainObject = facet;
		}
		
		key.resize(key.capacity());
		Dbt dbKey;
		initializeDbt(key, dbKey);
		
		value.resize(value.capacity());
		Dbt dbValue;
		initializeDbt(value, dbValue);

		//
		// Next facet
		//					    
		for(;;)
		{
		    try
		    {
			rs = dbc->get(&dbKey, &dbValue, DB_NEXT);
			if(rs == 0)
			{
			    key.resize(dbKey.get_size());
			    value.resize(dbValue.get_size());
			}
			break; // for(;;)
		    }
		    catch(const ::DbMemoryException dx)
		    {
			bool resized = false;
			if(dbKey.get_size() > dbKey.get_ulen())
			{
			    key.resize(dbKey.get_size());
			    initializeDbt(key, dbKey);
			    resized = true;
			}
			
			if(dbValue.get_size() > dbValue.get_ulen())
			{
			    value.resize(dbValue.get_size());
			    initializeDbt(value, dbValue);
			    resized = true;
			}
			
			if(!resized)
			{
			    //
			    // Real problem
			    //
			    DBException ex(__FILE__, __LINE__);
			    ex.message = dx.what();
			    throw ex;
			}
		    }
		}
	    } while(rs == 0 && startWith(key, root));

	    dbc->close();
	    break; // for (;;)
	       
	}
	catch(const ::DbDeadlockException&)
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
	catch(const ::DbException& dx)
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
	    DBException ex(__FILE__, __LINE__);
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

    //
    // Let's fix-up the facets tree in result
    //
    for(FacetMap::iterator q = result->facets.begin(); q != result->facets.end(); q++)
    {
	const FacetPath& facetPath = q->first;

	if(facetPath.size() > 0)
	{
	    FacetPath parent(facetPath);
	    parent.pop_back();
	    FacetMap::iterator r = result->facets.find(parent);
	    if(r == result->facets.end())
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
Freeze::EvictorI::addFacetImpl(EvictorElementPtr& element, const ObjectPtr& servant, const FacetPath& facetPath, bool replacing)
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
    Ice::ObjectPtr servant = 0; 

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


Ice::ObjectPtr
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
	//
	// Keep 0 length since we're not interested in the data
	//
	Dbt dbValue;
	dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
	
	for(;;)
	{
	    if(_key.size() < _key.capacity())
	    {
		_key.resize(_key.capacity());
	    }

	    Dbt dbKey;
	    initializeDbt(_key, dbKey);

	    try
	    {
		if(_dbc->get(&dbKey, &dbValue, DB_NEXT) == 0)
		{
		    _key.resize(dbKey.get_size());
		    unmarshal(_current, _key, _communicator);
		    
		    if(_current.facet.size() == 0)
		    {
			_currentSet = true;
			return true;
		    }
		    //
		    // Otherwise loop
		    //
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
	return _current.identity;
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

void
Freeze::EmptyFacetPathException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nempty facet path";
}
