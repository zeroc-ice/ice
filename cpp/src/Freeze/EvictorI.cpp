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

using namespace std;
using namespace Ice;
using namespace Freeze;

Freeze::EvictorI::EvictorI(const DBPtr& db, const CommunicatorPtr& communicator,
			   EvictorPersistenceMode persistenceMode) :
    _db(db),
    _evictorSize(10),
    _persistenceMode(persistenceMode),
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
    // Ignore requests to set the evictor size to values smaller than zero.
    //
    if (evictorSize < 0)
    {
	return;
    }

    //
    // Update the evictor size.
    //
    _evictorSize = static_cast<map<string, EvictorElementPtr>::size_type>(evictorSize);

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
Freeze::EvictorI::createObject(const string& identity, const ObjectPtr& servant)
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Save the new Ice Object to the database.
    //
    _db->putServant(identity, servant);
    add(identity, servant);

    if (_trace >= 1)
    {
	ostringstream s;
	s << "created \"" << identity << "\"";
	_logger->trace("Evictor", s.str());
    }

    //
    // Evict as many elements as necessary.
    //
    evict();
}

void
Freeze::EvictorI::destroyObject(const string& identity)
{
    JTCSyncT<JTCMutex> sync(*this);

    //
    // Delete the Ice Object from the database.
    //
    _db->delServant(identity);
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
Freeze::EvictorI::locate(const ObjectAdapterPtr& adapter, const string& identity, const string&,
			 LocalObjectPtr& cookie)
{
    JTCSyncT<JTCMutex> sync(*this);

    EvictorElementPtr element;

    map<string, EvictorElementPtr>::iterator p = _evictorMap.find(identity);
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
	element = p->second;
	_evictorList.erase(element->position);
	_evictorList.push_front(identity);
	element->position = _evictorList.begin();
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
	ObjectPtr servant = _db->getServant(identity);
	if (!servant)
	{
	    //
            // The Ice Object with the given identity does not exist,
            // client will get an ObjectNotExistException.
	    //
	    return 0;
	}
	
	//
	// Add the new Servant to the evictor queue.
	//
	element = add(identity, servant);

	//
	// If an initializer is installed, call it now.
	//
	if (_initializer)
	{
	    _initializer->initialize(adapter, identity, servant);
	}
    }

    //
    // Increase the usage count of the evictor queue element.
    //
    ++element->usageCount;

    //
    // Evict as many elements as necessary.
    //
    evict();

    //
    // Set the cookie and return the servant for the Ice Object.
    //
    cookie = element;
    return element->servant;
}

void
Freeze::EvictorI::finished(const ObjectAdapterPtr&, const string& identity, const ObjectPtr& servant,
			   const string& operation, const LocalObjectPtr& cookie)
{
    JTCSyncT<JTCMutex> sync(*this);

    assert(servant);

    //
    // Decrease the usage count of the evictor queue element.
    //
    EvictorElementPtr element = EvictorElementPtr::dynamicCast(cookie);
    assert(element);
    assert(element->usageCount >= 1);
    --element->usageCount;
    
    //
    // If we are in SaveAfterMutatingOperation mode, we must save the
    // Ice Object if this was a mutating call.
    //
    if (_persistenceMode == SaveAfterMutatingOperation)
    {
	if (servant->__isMutating(operation))
	{
	    _db->putServant(identity, servant);
	}
    }

    //
    // Evict as many elements as necessary.
    //
    evict();
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

    for (map<string, EvictorElementPtr>::iterator p = _evictorMap.begin(); p != _evictorMap.end(); ++p)
    {
	assert(p->second->usageCount == 0); // If the Servant is still in use, something is broken.

	//
	// If we are not in SaveAfterMutatingOperation mode, we must save
	// all Ice Objects in the database upon deactivation.
	//
	if (_persistenceMode != SaveAfterMutatingOperation)
	{
	    _db->putServant(*(p->second->position), p->second->servant);
	}
    }

    //
    // We must clear the evictor queue upon deactivation.
    //
    _evictorMap.clear();
    _evictorList.clear();
}

void
Freeze::EvictorI::evict()
{
    list<string>::reverse_iterator p = _evictorList.rbegin();

    //
    // With most STL implementations, _evictorMap.size() is faster
    // than _evictorList.size().
    //
    while (_evictorMap.size() > _evictorSize)
    {
	//
	// Get the last unused element from the evictor queue.
	//
	map<string, EvictorElementPtr>::iterator q;
	while(p != _evictorList.rend())
	{
	    q = _evictorMap.find(*p);
	    assert(q != _evictorMap.end());
	    if (q->second->usageCount == 0)
	    {
		break; // Fine, Servant is not in use.
	    }
	    ++p;
	}
	if(p == _evictorList.rend())
	{
	    //
	    // All Servants are active, can't evict any further.
	    //
	    break;
	}
	string identity = *p;
	EvictorElementPtr element = q->second;

	//
	// If we are in SaveUponEviction mode, we must save the Ice
	// Object that is about to be evicted to persistent store.
	//
	if (_persistenceMode == SaveUponEviction)
	{
	    _db->putServant(identity, element->servant);
	}

	//
	// Remove last unused element from the evictor queue.
	//
	_evictorList.erase(element->position);
	_evictorMap.erase(q);
	++p;

	if (_trace >= 2)
	{
	    ostringstream s;
	    s << "evicted \"" << identity << "\" from the queue\n"
	      << "number of elements in the queue: " << _evictorMap.size();
	    _logger->trace("Evictor", s.str());
	}
    }
}

Freeze::EvictorI::EvictorElementPtr
Freeze::EvictorI::add(const string& identity, const ObjectPtr& servant)
{
    //
    // Ignore the request if the Ice Object is already in the queue.
    //
    map<string, EvictorElementPtr>::const_iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	return p->second;
    }    

    //
    // Add an Ice Object with its Servant to the evictor queue.
    //
    _evictorList.push_front(identity);
    EvictorElementPtr element = new EvictorElement;
    element->servant = servant;
    element->position = _evictorList.begin();
    element->usageCount = 0;    
    _evictorMap[identity] = element;
    return element;
}

void
Freeze::EvictorI::remove(const string& identity)
{
    //
    // If the Ice Object is currently in the evictor, remove it.
    //
    map<string, EvictorElementPtr>::iterator p = _evictorMap.find(identity);
    if (p != _evictorMap.end())
    {
	_evictorList.erase(p->second->position);
	_evictorMap.erase(p);
    }
}
