// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Object.h> // Not included in Ice/Ice.h
#include <Freeze/EvictorI.h>
#include <sstream>

using namespace std;
using namespace Ice;
using namespace Freeze;

Freeze::EvictorI::EvictorI(const DBPtr& db, const CommunicatorPtr& communicator) :
    _db(db),
    _evictorSize(static_cast<map<string, EvictorElement>::size_type>(10)),
    _persistenceMode(SaveUponEviction),
    _logger(communicator->getLogger()),
    _trace(0)
{
    PropertiesPtr properties = communicator->getProperties();
    string value;

    value = properties->getProperty("Freeze.Trace.Evictor");
    if (!value.empty())
    {
	_trace = atoi(value.c_str());
    }
}

DBPtr
Freeze::EvictorI::getDB()
{
    //
    // No synchronizaton necessary, _db is immutable once the Evictor
    // has been created
    //
    return _db;
}

void
Freeze::EvictorI::setSize(Int evictorSize)
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Ignore requests to set the evictor size to values smaller or
    // equal to zero.
    //
    if (evictorSize <= 0)
    {
	return;
    }

    //
    // Update the evictor size.
    //
    _evictorSize = static_cast<map<string, EvictorElement>::size_type>(evictorSize);

    //
    // Evict as many elements as necessary.
    //
    evict();
}

Int
Freeze::EvictorI::getSize()
{
    JTCSyncT<JTCMutex> sync(*this);

    return static_cast<Int>(_evictorSize);
}

void
Freeze::EvictorI::setPersistenceMode(EvictorPersistenceMode persistenceMode)
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_persistenceMode != persistenceMode)
    {
	_persistenceMode = persistenceMode;

	//
	// If we switch to SaveAfterMutatingOperation, we save all
	// Servants now, so that it can safely be assumed that no data
	// can get lost after the mode has been changed.
	//
	if (_persistenceMode == SaveAfterMutatingOperation && !_evictorMap.empty())
	{
	    if (_trace >= 1)
	    {
		ostringstream s;
		s << "switching to SaveUponEviction\n"
		  << "saving all Ice Objects in the queue to the database";
		_logger->trace("Evictor", s.str());
	    }

	    for (map<string, EvictorElement>::iterator p = _evictorMap.begin(); p != _evictorMap.end(); ++p)
	    {
		_db->put(*(p->second.position), p->second.servant);
	    }
	}
    }
}

EvictorPersistenceMode
Freeze::EvictorI::getPersistenceMode()
{
    JTCSyncT<JTCMutex> sync(*this);

    return _persistenceMode;
}

void
Freeze::EvictorI::createObject(const string& identity, const ObjectPtr& servant)
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Save the new Ice Object to the database.
    //
    _db->put(identity, servant);
    add(identity, servant);

    if (_trace >= 1)
    {
	ostringstream s;
	s << "created \"" << identity << "\"";
	_logger->trace("Evictor", s.str());
    }
}

void
Freeze::EvictorI::destroyObject(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Delete the Ice Object from the database.
    //
    _db->del(identity);
    remove(identity);

    if (_trace >= 1)
    {
	ostringstream s;
	s << "destroyed \"" << identity << "\"";
	_logger->trace("Evictor", s.str());
    }
}

void
Freeze::EvictorI::installServantInitializer(const ServantInitializerPtr& initializer)
{
    JTCSyncT<JTCMutex> sync(*this);

    _initializer = initializer;
}

ObjectPtr
Freeze::EvictorI::locate(const ObjectAdapterPtr& adapter, const string& identity, const string&, ObjectPtr&)
{
    JTCSyncT<JTCMutex> sync(*this);
    
    map<string, EvictorElement>::iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	if (_trace >= 2)
	{
	    ostringstream s;
	    s << "found \"" << identity << "\" in the queue";
	    _logger->trace("Evictor", s.str());
	}

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
	if (_trace >= 2)
	{
	    ostringstream s;
	    s << "couldn't find \"" << identity << "\" in the queue\n"
	      << "loading \"" << identity << "\" from the database";
	    _logger->trace("Evictor", s.str());
	}

	//
	// Load the Ice Object from database and create and add a
	// Servant for it.
	//
	ObjectPtr servant = _db->get(identity);
	if (!servant)
	{
	    //
            // Ice Object with the given identity does not exist,
            // client will get an ObjectNotExistException.
	    //
	    return 0;
	}
	add(identity, servant);

	//
	// If an initializer is installed, call it now.
	//
	if (_initializer)
	{
	    _initializer->initialize(adapter, identity, servant);
	}

	//
	// Return the new servant for the Ice Object from the database.
	//
	return servant;
    }
}

void
Freeze::EvictorI::finished(const ObjectAdapterPtr&, const string& identity, const ObjectPtr& servant,
			   const string& operation, const ObjectPtr&)
{
    JTCSyncT<JTCMutex> sync(*this);

    assert(servant);

    //
    // If we are in SaveAfterMutatingOperation mode, we must save the
    // Ice Object if this was a mutating call.
    //
    if (_persistenceMode == SaveAfterMutatingOperation)
    {
	if (servant->__isMutating(operation))
	{
	    _db->put(identity, servant);
	}
    }
}

void
Freeze::EvictorI::deactivate()
{
    JTCSyncT<JTCMutex> sync(*this);

    if (_trace >= 1)
    {
	ostringstream s;
	s << "deactivating, saving all Ice Objects in the queue to the database";
	_logger->trace("Evictor", s.str());
    }

    //
    // If we are not in SaveAfterMutatingOperation mode, we must save
    // all Ice Objects in the database upon deactivation.
    //
    if (_persistenceMode != SaveAfterMutatingOperation)
    {
	for (map<string, EvictorElement>::iterator p = _evictorMap.begin(); p != _evictorMap.end(); ++p)
	{
	    _db->put(*(p->second.position), p->second.servant);
	}
    }

    //
    // We must clear the evictor map and list up deactivation.
    //
    _evictorMap.clear();
    _evictorList.clear();
}

void
Freeze::EvictorI::evict()
{
    //
    // With most STL implementations, _evictorMap.size() is faster
    // than _evictorList.size().
    //
    while (_evictorMap.size() > _evictorSize)
    {
	//
	// Get the last element of the Evictor queue.
	//
	string identity = _evictorList.back();
	map<string, EvictorElement>::iterator p = _evictorMap.find(identity);
	assert(p != _evictorMap.end());
	ObjectPtr servant = p->second.servant;
	assert(servant);

	//
	// Remove last element from the evictor queue.
	//
	_evictorMap.erase(identity);
	_evictorList.pop_back();
	assert(_evictorMap.size() == _evictorSize);

	//
	// If we are in SaveUponEviction mode, we must save the
	// evicted Ice Object to the database.
	//
	if (_persistenceMode == SaveUponEviction)
	{
	    _db->put(identity, servant);
	}

	if (_trace >= 2)
	{
	    ostringstream s;
	    s << "evicted \"" << identity << "\" from the queue\n"
	      << "number of elements in the queue: " << _evictorMap.size();
	    _logger->trace("Evictor", s.str());
	}
    }
}

void
Freeze::EvictorI::add(const string& identity, const ObjectPtr& servant)
{
    //
    // Ignore the request if the Ice Object is already in the queue.
    //
    if (_evictorMap.find(identity) != _evictorMap.end())
    {
	return;
    }    

    //
    // Add an Ice Object with its Servant to the evictor list and
    // evictor map.
    //
    _evictorList.push_front(identity);
    EvictorElement evictorElement;
    evictorElement.servant = servant;
    evictorElement.position = _evictorList.begin();
    _evictorMap[identity] = evictorElement;

    //
    // Evict as many elements as necessary.
    //
    evict();
}

void
Freeze::EvictorI::remove(const string& identity)
{
    //
    // If the Ice Object is currently in the evictor, remove it.
    //
    map<string, EvictorElement>::iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	_evictorList.erase(p->second.position);
	_evictorMap.erase(p);
    }
}
