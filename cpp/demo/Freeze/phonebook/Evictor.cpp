// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Evictor.h>
#include <PhoneBookI.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace Freeze;

Evictor::Evictor(const DBPtr& db, Int size) :
    _db(db),
    _evictorSize(static_cast<map<string, EvictorEntry>::size_type>(size))
{
}

void
Evictor::createObject(const string& identity, const ObjectPtr& servant)
{
    //
    // Save the new Ice Object to the database.
    //
    _db->put(identity, servant);

    //
    // Evict one element, if necessary, to make room for a new
    // one. Then add the new Ice Object to the evictor.
    //
    evict();
    add(identity, servant);
}

void
Evictor::destroyObject(const string& identity)
{
    //
    // Delete the Ice Object from the database.
    //
    _db->del(identity);

    //
    // If the Ice Object is currently in the evictor, remove it.
    //
    map<string, EvictorEntry>::iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	_evictorList.erase(p->second.position);
	_evictorMap.erase(p);
    }
}

ObjectPtr
Evictor::locate(const ObjectAdapterPtr&, const string& identity, ObjectPtr&)
{
    JTCSyncT<JTCMutex> sync(*this);
    
    map<string, EvictorEntry>::iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	//
	// Ice Object found in evictor map. Push it to the front of
	// the evictor list, so that it will be evicted last.
	//
	_evictorList.erase(p->second.position);
	_evictorList.push_front(identity);
	p->second.position = _evictorList.begin();

	//
	// Return the servant for the Ice Object.
	//
	return p->second.servant;
    }
    else
    {
	//
	// Evict one element, if necessary, to make room for a new
	// one.
	//
	evict();

	//
	// Find the Ice Object in the database and create a servant
	// for it.
	//
	ObjectPtr servant = _db->get(identity);
	if (!servant)
	{
	    //
            // Ice object with the given identity does not exist,
            // client will get an ObjectNotExistException.
	    //
	    return 0;
	}

	//
	// TODO: That's the only PhoneBook specific stuff!
	//
	EntryIPtr entry = EntryIPtr::dynamicCast(servant);
	assert(entry);
	entry->setIdentity(identity);

	//
	// Add the new Ice Object and its Servant
	//
	add(identity, servant);

	//
	// Return the new servant for the Ice Object from the database.
	//
	return servant;
    }
}

void
Evictor::finished(const ObjectAdapterPtr&, const string&, const ObjectPtr&, const ObjectPtr&)
{
    //JTCSyncT<JTCMutex> sync(*this);
}

void
Evictor::deactivate()
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Save all Ice Objects in the database upon deactivation, and
    // clear the evictor map and list.
    //
    for (map<string, EvictorEntry>::iterator p = _evictorMap.begin(); p != _evictorMap.end(); ++p)
    {
	_db->put(*(p->second.position), p->second.servant);
    }
    _evictorMap.clear();
    _evictorList.clear();
}

void
Evictor::evict()
{
    //
    // With most STL implementations, _evictorMap.size() is faster
    // than _evictorList.size().
    //
    if (_evictorMap.size() > _evictorSize)
    {
	//
	// Evictor size exceeded. Remove last element from the evictor list.
	//
	string identity = _evictorList.back();
	_evictorList.pop_back();

	//
	// Find corresponding element in the evictor map
	//
	map<string, EvictorEntry>::iterator p = _evictorMap.find(identity);
	assert(p != _evictorMap.end());

	//
	// Save the evicted Ice Object to the database.
	//
	_db->put(identity, p->second.servant);

	//
	// Remove the element from the evictor map.
	//
	_evictorMap.erase(identity);
	assert(_evictorMap.size() == _evictorSize);
    }
}

void
Evictor::add(const string& identity, const ObjectPtr& servant)
{
    //
    // Add an Ice Object with its Servant to the evictor list and
    // evictor map.
    //
    _evictorList.push_front(identity);
    EvictorEntry evictorEntry;
    evictorEntry.servant = servant;
    evictorEntry.position = _evictorList.begin();
    _evictorMap[identity] = evictorEntry;
}
