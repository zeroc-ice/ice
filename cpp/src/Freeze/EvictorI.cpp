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

using namespace std;
using namespace Ice;
using namespace Freeze;

namespace Freeze
{

class EvictorIteratorI : public EvictorIterator
{
public:

    EvictorIteratorI(const IdentityObjectDict::const_iterator&, const IdentityObjectDict::const_iterator&);
    virtual ~EvictorIteratorI();

    virtual bool hasNext();
    virtual Ice::Identity next();
    virtual void destroy();

private:

    IdentityObjectDict::const_iterator _curr;
    IdentityObjectDict::const_iterator _end;
};

}

Freeze::EvictorI::EvictorI(const DBPtr& db, const PersistenceStrategyPtr& strategy) :
    _evictorSize(10),
    _deactivated(false),
    _dict(db),
    _db(db),
    _strategy(strategy),
    _trace(0)
{
    _trace = _db->getCommunicator()->getProperties()->getPropertyAsInt("Freeze.Trace.Evictor");
}

Freeze::EvictorI::~EvictorI()
{
    if(!_deactivated)
    {
	Warning out(_db->getCommunicator()->getLogger());
	out << "evictor has not been deactivated";
    }

    _strategy->destroy();
}

DBPtr
Freeze::EvictorI::getDB()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return _db;
}

PersistenceStrategyPtr
Freeze::EvictorI::getPersistenceStrategy()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return _strategy;
}

void
Freeze::EvictorI::setSize(Int evictorSize)
{
    IceUtil::Mutex::Lock sync(*this);

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

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return static_cast<Int>(_evictorSize);
}

void
Freeze::EvictorI::createObject(const Identity& ident, const ObjectPtr& servant)
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    //
    // Save the Ice object's initial state and add it to the queue.
    //

    save(ident, servant);
    add(ident, servant);

    if(_trace >= 1)
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

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    //
    // Delete the Ice object from the database.
    //
    _dict.erase(ident);

    EvictorElementPtr element = remove(ident);
    if(element)
    {
        element->destroyed = true;

        //
        // Notify the persistence strategy.
        //
        _strategy->destroyedObject(ident, element->strategyCookie);

        if(_trace >= 1)
        {
            Trace out(_db->getCommunicator()->getLogger(), "Evictor");
            out << "destroyed \"" << ident << "\"";
        }
    }
}

void
Freeze::EvictorI::installServantInitializer(const ServantInitializerPtr& initializer)
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    _initializer = initializer;
}

EvictorIteratorPtr
Freeze::EvictorI::getIterator()
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return new EvictorIteratorI(_dict.begin(), _dict.end());
}

bool
Freeze::EvictorI::hasObject(const Ice::Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

    return _dict.find(ident) != _dict.end();
}

ObjectPtr
Freeze::EvictorI::locate(const Current& current, LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_db);

    //
    // If this operation is called on a deactivated servant locator,
    // it's a bug in Ice.
    //
    assert(!_deactivated);

    EvictorElementPtr element;

    map<Identity, EvictorElementPtr>::iterator p = _evictorMap.find(current.id);
    if(p != _evictorMap.end())
    {
	if(_trace >= 2)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "found \"" << current.id << "\" in the queue";
	}

	//
	// Ice object found in evictor map. Push it to the front of
	// the evictor list, so that it will be evicted last.
	//
	element = p->second;
	_evictorList.erase(element->position);
	_evictorList.push_front(current.id);
	element->position = _evictorList.begin();
    }
    else
    {
	if(_trace >= 2)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "couldn't find \"" << current.id << "\" in the queue\n"
		<< "loading \"" << current.id << "\" from the database";
	}

	//
	// Load the Ice object from the database and add a
        // servant for it.
	//
	IdentityObjectDict::iterator p = _dict.find(current.id);
	if(p == _dict.end())
	{
	    //
            // The Ice object with the given identity does not exist,
            // client will get an ObjectNotExistException.
	    //
	    return 0;
	}

	//
	// If an initializer is installed, call it now.
	//
	ObjectPtr servant = p->second;
	if(_initializer)
	{
	    _initializer->initialize(current.adapter, current.id, servant);
	}

	//
	// Add the new servant to the evictor queue.
	//
	element = add(current.id, servant);
    }

    //
    // Increase the usage count of the evictor queue element.
    //
    ++element->usageCount;
    if(current.mode != Nonmutating)
    {
        ++element->mutatingCount;
    }

    //
    // Evict as many elements as necessary.
    //
    evict();

    //
    // Set the cookie and return the servant for the Ice object.
    //
    cookie = element;
    return element->servant;
}

void
Freeze::EvictorI::finished(const Current& current, const ObjectPtr& servant, const LocalObjectPtr& cookie)
{
    IceUtil::Mutex::Lock sync(*this);

    assert(_db);
    assert(servant);

    //
    // It's possible that the locator has been deactivated already. In
    // this case, _evictorSize is set to zero.
    //
    assert(!_deactivated || !_evictorSize);

    //
    // Decrease the usage count of the evictor queue element.
    //
    EvictorElementPtr element = EvictorElementPtr::dynamicCast(cookie);
    assert(element);
    assert(element->usageCount >= 1);
    --element->usageCount;
    if(current.mode != Nonmutating)
    {
        assert(element->mutatingCount >= 1);
        --element->mutatingCount;
    }

    //
    // If the object has not been destroyed, notify the persistence
    // strategy about a mutating operation.
    //
    if(!element->destroyed)
    {
        _strategy->invokedObject(this, current.id, servant, current.mode != Nonmutating, element->mutatingCount == 0,
                                 element->strategyCookie);
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

    if(!_deactivated)
    {
	_deactivated = true;
	
	if(_trace >= 1)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "deactivating, saving unsaved Ice objects to the database";
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
Freeze::EvictorI::save(const Identity& ident, const ObjectPtr& servant)
{
    //
    // NOTE: Do not synchronize on the evictor mutex or else
    // deadlocks may occur.
    //
    _dict.insert(pair<const Identity, const ObjectPtr>(ident, servant));
}

void
Freeze::EvictorI::evict()
{
    list<Identity>::reverse_iterator p = _evictorList.rbegin();

    //
    // With most STL implementations, _evictorMap.size() is faster
    // than _evictorList.size().
    //
    while(_evictorMap.size() > _evictorSize)
    {
	//
	// Get the last unused element from the evictor queue.
	//
	map<Identity, EvictorElementPtr>::iterator q;
	while(p != _evictorList.rend())
	{
	    q = _evictorMap.find(*p);
	    assert(q != _evictorMap.end());
	    if(q->second->usageCount == 0)
	    {
		break; // Fine, servant is not in use.
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
	Identity ident = *p;
	EvictorElementPtr element = q->second;


        //
        // Notify the persistence strategy about the evicted object.
        //
        _strategy->evictedObject(this, ident, element->servant, element->strategyCookie);

	//
	// Remove last unused element from the evictor queue.
	//
	assert(--(p.base()) == element->position);
	p = list<Identity>::reverse_iterator(_evictorList.erase(element->position));
	_evictorMap.erase(q);

	if(_trace >= 2)
	{
	    Trace out(_db->getCommunicator()->getLogger(), "Evictor");
	    out << "evicted \"" << ident << "\" from the queue\n"
		<< "number of elements in the queue: " << _evictorMap.size();
	}
    }

    //
    // If we're deactivated, and if there are no more elements to
    // evict, set _db to zero to break cyclic object dependencies.
    //
    if(_deactivated && _evictorMap.empty())
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
    // Ignore the request if the Ice object is already in the queue.
    //
    map<Identity, EvictorElementPtr>::const_iterator p = _evictorMap.find(ident);
    if(p != _evictorMap.end())
    {
	return p->second;
    }    

    //
    // Add an Ice object with its servant to the evictor queue.
    //
    _evictorList.push_front(ident);

    EvictorElementPtr element = new EvictorElement;
    element->servant = servant;
    element->position = _evictorList.begin();
    element->usageCount = 0;
    element->mutatingCount = 0;
    element->destroyed = false;
    element->strategyCookie = _strategy->activatedObject(ident, servant);

    _evictorMap[ident] = element;
    return element;
}

Freeze::EvictorI::EvictorElementPtr
Freeze::EvictorI::remove(const Identity& ident)
{
    //
    // If the Ice object is currently in the evictor, remove it.
    //
    map<Identity, EvictorElementPtr>::iterator p = _evictorMap.find(ident);
    EvictorElementPtr element;
    if(p != _evictorMap.end())
    {
        element = p->second;
	_evictorList.erase(element->position);
	_evictorMap.erase(p);
    }
    return element;
}

void
Freeze::EvictorDeactivatedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
}

Freeze::EvictorIteratorI::EvictorIteratorI(const IdentityObjectDict::const_iterator& begin,
					   const IdentityObjectDict::const_iterator& end) :
    _curr(begin),
    _end(end)
{
}

Freeze::EvictorIteratorI::~EvictorIteratorI()
{
}

bool
Freeze::EvictorIteratorI::hasNext()
{
    return _curr != _end;
}

Ice::Identity
Freeze::EvictorIteratorI::next()
{
    if(_curr == _end)
    {
	throw Freeze::NoSuchElementException(__FILE__, __LINE__);
    }

    Ice::Identity ident = _curr->first;
    ++_curr;
    return ident;
}

void
Freeze::EvictorIteratorI::destroy()
{
    //
    // Set the current element to the end.
    //
    _curr = _end;
}

//
// Print for the various exception types.
//
void
Freeze::NoSuchElementException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nno such element";
}
