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

    EvictorIteratorI(const IdentityObjectRecordDict::const_iterator&, const IdentityObjectRecordDict::const_iterator&);
    virtual ~EvictorIteratorI();

    virtual bool hasNext();
    virtual Ice::Identity next();
    virtual void destroy();

private:

    IdentityObjectRecordDict::const_iterator _curr;
    IdentityObjectRecordDict::const_iterator _end;
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

    ObjectRecord rec;
    rec.servant = servant;
    rec.stats.creationTime = IceUtil::Time::now().toMilliSeconds();
    rec.stats.lastSaveTime = 0;
    rec.stats.avgSaveTime = 0;

    //
    // Save the Ice object's initial state and add it to the queue.
    //
    _dict.put(pair<const Identity, const ObjectRecord>(ident, rec));
    add(ident, rec);

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
Freeze::EvictorI::saveObject(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);
    
    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }
    
    map<Identity, EvictorElementPtr>::iterator p = _evictorMap.find(ident);
    if(p == _evictorMap.end())
    {
	throw ObjectDestroyedException(__FILE__, __LINE__);
    }
    EvictorElementPtr element = p->second;
    assert(!element->destroyed);
    
    //
    // TODO: another lookup is not very efficient! 
    //
    save(ident, element->rec.servant);
    
    _strategy->savedObject(this, ident, element->rec.servant, element->strategyCookie, element->usageCount);
}

void
Freeze::EvictorI::destroyObject(const Identity& ident)
{
    IceUtil::Mutex::Lock sync(*this);

    if(_deactivated)
    {
	throw EvictorDeactivatedException(__FILE__, __LINE__);
    }

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

    //
    // Delete the Ice object from the database.
    //
    _dict.erase(ident);
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
	IdentityObjectRecordDict::iterator q = _dict.find(current.id);
	if(q == _dict.end())
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
	if(_initializer)
	{
	    _initializer->initialize(current.adapter, current.id, q->second.servant);
	}

	//
	// Add the new servant to the evictor queue.
	//
	element = add(current.id, q->second);
    }

    //
    // Increase the usage count of the evictor queue element.
    //
    ++element->usageCount;

    //
    // Notify the persistence strategy about the operation.
    //
    _strategy->preOperation(this, current.id, element->rec.servant, current.mode != Nonmutating,
                            element->strategyCookie);

    //
    // Evict as many elements as necessary.
    //
    evict();

    //
    // Set the cookie and return the servant for the Ice object.
    //
    cookie = element;
    return element->rec.servant;
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

    //
    // If the object has not been destroyed, notify the persistence
    // strategy about a mutating operation.
    //
    if(!element->destroyed)
    {
        _strategy->postOperation(this, current.id, servant, current.mode != Nonmutating, element->strategyCookie);
    }

    //
    // Evict as many elements as necessary.
    //
    evict();
}

void
Freeze::EvictorI::deactivate(const string&)
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
    // NOTE: This operation is not mutex-protected, therefore it may
    // only be invoked while the evictor is already locked. For
    // example, it is safe to call this operation from a persistence
    // strategy implementation, iff the persistence strategy is in
    // the thread context of a locked evictor operation.
    //
    map<Identity, EvictorElementPtr>::iterator p = _evictorMap.find(ident);
    assert(p != _evictorMap.end());
    EvictorElementPtr element = p->second;

    //
    // Update statistics before saving.
    //
    Long now = IceUtil::Time::now().toMilliSeconds();
    Long diff = now - (element->rec.stats.creationTime + element->rec.stats.lastSaveTime);
    if(element->rec.stats.lastSaveTime == 0)
    {
        element->rec.stats.lastSaveTime = diff;
        element->rec.stats.avgSaveTime = diff;
    }
    else
    {
        element->rec.stats.lastSaveTime = now - element->rec.stats.creationTime;
        element->rec.stats.avgSaveTime = (Long)(element->rec.stats.avgSaveTime * 0.95 + diff * 0.05);
    }

    _dict.put(pair<const Identity, const ObjectRecord>(ident, element->rec));
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
        _strategy->evictedObject(this, ident, element->rec.servant, element->strategyCookie);

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
Freeze::EvictorI::add(const Identity& ident, const ObjectRecord& rec)
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
    element->rec = rec;
    element->position = _evictorList.begin();
    element->usageCount = 0;
    element->destroyed = false;
    element->strategyCookie = _strategy->activatedObject(ident, rec.servant);

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

void
Freeze::ObjectDestroyedException::ice_print(ostream& out) const
{
    Exception::ice_print(out);
    out << ":\nunknown local exception";
}

Freeze::EvictorIteratorI::EvictorIteratorI(const IdentityObjectRecordDict::const_iterator& begin,
					   const IdentityObjectRecordDict::const_iterator& end) :
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
