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

Freeze::EvictorI::EvictorI(const DBPtr& db, EvictorPersistenceMode persistenceMode) :
    _evictorSize(10),
    _deactivated(false),
    _dict(db),
    _db(db),
    _persistenceMode(persistenceMode),
    _trace(0)
{
    _trace = _db->getCommunicator()->getProperties()->getPropertyAsInt("Freeze.Trace.Evictor");
}

Freeze::EvictorI::~EvictorI()
{
    if (!_deactivated)
    {
	Warning out(_db->getCommunicator()->getLogger());
	out << "evictor has not been deactivated";
    }
}

DBPtr
Freeze::EvictorI::getDB()
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return _db;
}

void
Freeze::EvictorI::setSize(Int evictorSize)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

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
    _evictorSize = static_cast<map<Identity, EvictorElementPtr>::size_type>(evictorSize);

    //
    // Evict as many elements as necessary.
    //
    evict();
}

Int
Freeze::EvictorI::getSize()
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return static_cast<Int>(_evictorSize);
}

void
Freeze::EvictorI::createObject(const Identity& ident, const ObjectPtr& servant)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    //
    // Save the new Ice Object to the database.
    //
    _dict.insert(make_pair(ident, servant));
    add(ident, servant);

    if (_trace >= 1)
    {
	Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	out << "created \"" << ident << "\"";
    }

    //
    // Evict as many elements as necessary.
    //
    evict();
}

void
Freeze::EvictorI::destroyObject(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    //
    // Delete the Ice Object from the database.
    //
    _dict.erase(ident);
    remove(ident);

    if (_trace >= 1)
    {
	Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	out << "destroyed \"" << ident << "\"";
    }
}

void
Freeze::EvictorI::installServantInitializer(const ServantInitializerPtr& initializer)
{
    IceUtil::Mutex::Lock sync(*this);

    if (_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    _initializer = initializer;
}

ObjectPtr
Freeze::EvictorI::locate(const ObjectAdapterPtr& adapter, const Current& current, LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_db);

    //
    // If this operation is called on a deactivated servant locator,
    // it's a bug in Ice.
    //
    assert(!_deactivated);

    EvictorElementPtr element;

    map<Identity, EvictorElementPtr>::iterator p = _evictorMap.find(current.identity);
    if (p != _evictorMap.end())
    {
	if (_trace >= 2)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "found \"" << current.identity << "\" in the queue";
	}

	//
	// Ice Object found in evictor map. Push it to the front of
	// the evictor list, so that it will be evicted last.
	//
	element = p->second;
	_evictorList.erase(element->position);
	_evictorList.push_front(current.identity);
	element->position = _evictorList.begin();
    }
    else
    {
	if (_trace >= 2)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "couldn't find \"" << current.identity << "\" in the queue\n"
		<< "loading \"" << current.identity << "\" from the database";
	}

	//
	// Load the Ice Object from database and create and add a
	// Servant for it.
	//
	IdentityObjectDict::iterator p = _dict.find(current.identity);
	if (p == _dict.end())
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
	ObjectPtr servant = p->second;
	element = add(current.identity, servant);

	//
	// If an initializer is installed, call it now.
	//
	if (_initializer)
	{
	    _initializer->initialize(adapter, current.identity, servant);
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
Freeze::EvictorI::finished(const ObjectAdapterPtr&, const Current& current,
			   const ObjectPtr& servant, const LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_db);
    assert(servant);

    //
    // It's possible that the locator has been deactivated already. In
    // this case, _evictorSize is set to zero.
    //
    assert(!_deactivated || _evictorSize);

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
	if (!current.nonmutating)
	{
	    _dict.insert(make_pair(current.identity, servant));
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
    IceUtil::Mutex::Lock sync(*this);

    if (!_deactivated)
    {
	_deactivated = true;
	
	if (_trace >= 1)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "deactivating, saving unsaved Ice Objects to the database";
	}

	//
	// Set the evictor size to zero, meaning that we will evict
	// everything possible.
	//
	_evictorSize = 0;
	evict();
    }
}

void
Freeze::EvictorI::evict()
{
    list<Identity>::reverse_iterator p = _evictorList.rbegin();

    //
    // With most STL implementations, _evictorMap.size() is faster
    // than _evictorList.size().
    //
    while (_evictorMap.size() > _evictorSize)
    {
	//
	// Get the last unused element from the evictor queue.
	//
	map<Identity, EvictorElementPtr>::iterator q;
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
	Identity ident = *p;
	EvictorElementPtr element = q->second;

	//
	// If we are in SaveUponEviction mode, we must save the Ice
	// Object that is about to be evicted to persistent store.
	//
	if (_persistenceMode == SaveUponEviction)
	{
	    _dict.insert(make_pair(ident, element->servant));
	}

	//
	// Remove last unused element from the evictor queue.
	//
	_evictorList.erase(element->position);
	_evictorMap.erase(q);
	++p;

	if (_trace >= 2)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "evicted \"" << ident << "\" from the queue\n"
		<< "number of elements in the queue: " << _evictorMap.size();
	}
    }

    //
    // If we're deactivated, and if there are no more elements to
    // evict, set _db to zero to break cyclic object
    // dependencies.
    //
    if (_deactivated && _evictorMap.empty())
    {
	assert(_evictorList.empty());
	assert(_evictorSize == 0);
	_db = 0;
    }
}

Freeze::EvictorI::EvictorElementPtr
Freeze::EvictorI::add(const Identity& ident, const ObjectPtr& servant)
{
    //
    // Ignore the request if the Ice Object is already in the queue.
    //
    map<Identity, EvictorElementPtr>::const_iterator p = _evictorMap.find(ident);
    if (p != _evictorMap.end())
    {
	return p->second;
    }    

    //
    // Add an Ice Object with its Servant to the evictor queue.
    //
    _evictorList.push_front(ident);
    EvictorElementPtr element = new EvictorElement;
    element->servant = servant;
    element->position = _evictorList.begin();
    element->usageCount = 0;    
    _evictorMap[ident] = element;
    return element;
}

void
Freeze::EvictorI::remove(const Identity& ident)
{
    //
    // If the Ice Object is currently in the evictor, remove it.
    //
    map<Identity, EvictorElementPtr>::iterator p = _evictorMap.find(ident);
    if (p != _evictorMap.end())
    {
	_evictorList.erase(p->second->position);
	_evictorMap.erase(p);
    }
}

void
Freeze::EvictorDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
}
