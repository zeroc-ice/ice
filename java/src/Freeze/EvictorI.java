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

package Freeze;

class EvictorI extends Ice.LocalObjectImpl implements Evictor, ObjectStore
{
    synchronized public DB
    getDB()
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	return _db;
    }

    synchronized public PersistenceStrategy
    getPersistenceStrategy()
    {
        if(_deactivated)
        {
            throw new EvictorDeactivatedException();
        }

        return _strategy;
    }
    
    synchronized public void
    setSize(int evictorSize)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
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
	_evictorSize = evictorSize;
	
	//
	// Evict as many elements as necessary.
	//
	evict();
    }

    synchronized public int
    getSize()
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}
	
	return _evictorSize;
    }
    
    synchronized public void
    createObject(Ice.Identity ident, Ice.Object servant)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	//
	// Copy the identity. This is necessary for add().
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;

        ObjectRecord rec = new ObjectRecord();
        rec.servant = servant;
        rec.stats = new Statistics();
        rec.stats.creationTime = System.currentTimeMillis();
        rec.stats.lastSaveTime = 0;
        rec.stats.avgSaveTime = 0;

        //
        // Save the Ice object's initial state and add it to the queue.
        //
        _dict.fastPut(ident, rec);
	add(identCopy, rec);

	if(_trace >= 1)
	{
	    _db.getCommunicator().getLogger().trace("Freeze::Evictor",
	                                            "created \"" + Ice.Util.identityToString(ident) + "\"");
	}
	
	//
	// Evict as many elements as necessary.
	//
	evict();
    }

    synchronized public void
    saveObject(Ice.Identity ident)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	if(element == null)
	{
	    throw new ObjectDestroyedException();
	}
	assert(!element.destroyed);

	save(ident, element.rec.servant);
	_strategy.savedObject(this, ident, element.rec.servant, element.strategyCookie, element.usageCount);
    }


    synchronized public void
    destroyObject(Ice.Identity ident)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	EvictorElement element = remove(ident);
        if(element != null)
	{
	    element.destroyed = true;

            //
            // Notify the persistence strategy.
            //
            _strategy.destroyedObject(ident, element.strategyCookie);

            if(_trace >= 1)
            {
                _db.getCommunicator().getLogger().trace("Freeze::Evictor", "destroyed \"" +
                                                        Ice.Util.identityToString(ident) + "\"");
            }
	}

	//
	// Delete the Ice object from the database.
	//
        _dict.fastRemove(ident);
    }
    
    synchronized public void
    installServantInitializer(ServantInitializer initializer)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}
	
	_initializer = initializer;
    }

    synchronized public EvictorIterator
    getIterator()
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	return new EvictorIteratorI(_dict.entrySet().iterator());
    }

    synchronized public boolean
    hasObject(Ice.Identity ident)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}
	
	return _dict.containsKey(ident);
    }

    synchronized public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
	assert(_db != null);
	
	//
	// If this operation is called on a deactivated servant locator,
	// it's a bug in Ice.
	//
	assert(!_deactivated);

	//
	// First copy current.identity. This is necessary since this
	// identity is later added to the evictor list (and
	// potentially the map).
	//
	Ice.Identity ident = new Ice.Identity();
	ident.name = current.id.name;
	ident.category = current.id.category;
	
	EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	if(element != null)
	{
	    if(_trace >= 2)
	    {
		_db.getCommunicator().getLogger().trace("Freeze::Evictor",
							"found \"" + Ice.Util.identityToString(ident) +
							"\" in the queue");
	    }
	    
	    //
	    // Ice object found in evictor map. Push it to the front of
	    // the evictor list, so that it will be evicted last.
	    //
	    element.position.remove();
	    _evictorList.addFirst(ident);
	    element.position = _evictorList.iterator();

	    //
	    // Position the iterator "on" the element.
	    //
	    element.position.next();
	}
	else
	{
	    if(_trace >= 2)
	    {
		_db.getCommunicator().getLogger().trace(
		    "Freeze::Evictor",
		    "couldn't find \"" + Ice.Util.identityToString(ident) + "\" in the queue\n"
		    + "loading \"" + Ice.Util.identityToString(ident) + "\" from the database");
	    }
	    
	    //
	    // Load the Ice object from the database and add a
	    // servant for it.
	    //
	    ObjectRecord rec = (ObjectRecord)_dict.get(ident);
	    if(rec == null)
	    {
		//
		// The Ice object with the given identity does not exist,
		// client will get an ObjectNotExistException.
		//
		return null;
	    }

	    //
	    // If an initializer is installed, call it now.
	    //
	    if(_initializer != null)
	    {
		_initializer.initialize(current.adapter, ident, rec.servant);
	    }

	    //
	    // Add the new servant to the evictor queue.
	    //
	    element = add(ident, rec);
	}
	
	//
	// Increase the usage count of the evictor queue element.
	//
	++element.usageCount;

        //
        // Notify the persistence strategy about the operation.
        //
        _strategy.preOperation(this, ident, element.rec.servant, current.mode != Ice.OperationMode.Nonmutating,
                               element.strategyCookie);

	//
	// Evict as many elements as necessary.
	//
	evict();
	
	//
	// Set the cookie and return the servant for the Ice object.
	//
	cookie.value = (Ice.LocalObject)element;
	return element.rec.servant;
    }

    synchronized public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
	assert(_db != null);
	assert(servant != null);
	
	//
	// It's possible that the locator has been deactivated already. In
	// this case, _evictorSize is set to zero.
	//
	assert(!_deactivated || _evictorSize == 0);
	
	//
	// Decrease the usage count of the evictor queue element.
	//
	EvictorElement element = (EvictorElement)cookie;
	assert(element.usageCount >= 1);
	--element.usageCount;

        //
        // If the object has not been destroyed, notify the persistence
        // strategy about the operation.
        //
        if(!element.destroyed)
        {
            _strategy.postOperation(this, current.id, servant, current.mode != Ice.OperationMode.Nonmutating,
                                    element.strategyCookie);
        }
	
	//
	// Evict as many elements as necessary.
	//
	evict();
    }

    synchronized public void
    deactivate(String category)
    {
	if(!_deactivated)
	{
	    _deactivated = true;
	    
	    if(_trace >= 1)
	    {
		_db.getCommunicator().getLogger().trace("Freeze::Evictor",
							"deactivating, saving unsaved Ice objects to the database");
	    }
	    
	    //
	    // Set the evictor size to zero, meaning that we will evict
	    // everything possible.
	    //
	    _evictorSize = 0;
	    evict();
	}
    }

    public void
    save(Ice.Identity ident, Ice.Object servant)
    {
        //
        // NOTE: This operation is not mutex-protected, therefore it may
        // only be invoked while the evictor is already locked. For
        // example, it is safe to call this operation from a persistence
        // strategy implementation, iff the persistence strategy is in
        // the thread context of a locked evictor operation.
        //
        EvictorElement element = (EvictorElement)_evictorMap.get(ident);
        assert(element != null);

        //
        // Update statistics before saving.
        //
        long now = System.currentTimeMillis();
        long diff = now - (element.rec.stats.creationTime + element.rec.stats.lastSaveTime);
        if(element.rec.stats.lastSaveTime == 0)
        {
            element.rec.stats.lastSaveTime = diff;
            element.rec.stats.avgSaveTime = diff;
        }
        else
        {
            element.rec.stats.lastSaveTime = now - element.rec.stats.creationTime;
            element.rec.stats.avgSaveTime = (long)(element.rec.stats.avgSaveTime * 0.95 + diff * 0.05);
        }

        _dict.fastPut(ident, element.rec);
    }

    EvictorI(DB db, PersistenceStrategy strategy)
    {
	_db = db;
	_dict = new IdentityObjectRecordDict(db);
        _strategy = strategy;
	_trace = _db.getCommunicator().getProperties().getPropertyAsInt("Freeze.Trace.Evictor");
    }

    protected void
    finalize()
        throws Throwable
    {
        if(!_deactivated)
        {
            _db.getCommunicator().getLogger().warning("evictor has not been deactivated");
        }

        _strategy.destroy();
    }

    private void
    evict()
    {
	java.util.Iterator p = _evictorList.riterator();
	while(p.hasNext() && _evictorList.size() > _evictorSize)
	{
	    //
	    // Get the last unused element from the evictor queue.
	    //
	    Ice.Identity ident = (Ice.Identity)p.next();
	    EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	    assert(element != null);
	    if(element.usageCount == 0)
	    {
		//
		// Fine, servant is not in use.
		//
		assert(ident != null && element != null);

                //
                // Notify the persistence strategy about the evicted object.
                //
                _strategy.evictedObject(this, ident, element.rec.servant, element.strategyCookie);

		//
		// Remove element from the evictor queue.
		//
		p.remove();
		_evictorMap.remove(ident);

		if(_trace >= 2)
		{
		    _db.getCommunicator().getLogger().trace(
			"Freeze::Evictor", 
			"evicted \"" + Ice.Util.identityToString(ident) +
			"\" from the queue\n" + "number of elements in the queue: " +
			_evictorMap.size());
		}
	    }
	}

	//
	// If we're deactivated and there are no more elements to
	// evict, it's not necessary in Java to set _db to zero to
	// break cyclic dependencies.
	//
    }

    private EvictorElement
    add(Ice.Identity ident, ObjectRecord rec)
    {
	//
	// Ignore the request if the Ice object is already in the queue.
	//
	EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	if(element != null)
	{
	    return element;
	}    
	
	//
	// Add an Ice object with its servant to the evictor queue.
	//
	_evictorList.addFirst(ident);

	element = new EvictorElement();
        element.rec = rec;
	element.position = _evictorList.iterator();
	element.usageCount = 0;    
	element.destroyed = false;    
        element.strategyCookie = _strategy.activatedObject(ident, rec.servant);

	//
	// Position the iterator "on" the element.
	//
	element.position.next();

	_evictorMap.put(ident, element);
	return element;
    }

    private EvictorElement
    remove(Ice.Identity ident)
    {
	//
	// If the Ice object is currently in the evictor, remove it.
	//
	EvictorElement element = (EvictorElement)_evictorMap.remove(ident);
	if(element != null)
	{
	    element.position.remove();
	}
        return element;
    }

    class EvictorElement extends Ice.LocalObjectImpl
    {
        ObjectRecord rec;
	java.util.Iterator position;
	int usageCount;
        boolean destroyed;
        Ice.LocalObject strategyCookie;
    };

    //
    // Map of Ice.Identity to EvictorElement
    //
    private java.util.Map _evictorMap = new java.util.HashMap();

    //
    // The C++ Evictor uses std::list<Ice::Identity> which allows
    // holding of iterators across list changes. Unfortunately, Java
    // iterators are invalidated as soon as the underlying collection
    // is changed, so it's not possible to use the same technique.
    //
    // This is a list of Ice.Identity.
    //
    private IceInternal.LinkedList _evictorList = new IceInternal.LinkedList();

    private int _evictorSize = 10;
    private boolean _deactivated = false;
    private IdentityObjectRecordDict _dict;

    private DB _db;
    private PersistenceStrategy _strategy;
    private ServantInitializer _initializer;
    private int _trace = 0;
}
