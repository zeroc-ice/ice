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

        //
        // Save the Ice object's initial state and add it to the queue.
        //
        save(identCopy, servant);
	add(identCopy, servant);
	
	if(_trace >= 1)
	{
	    _db.getCommunicator().getLogger().trace("Evictor", "created \"" + Ice.Util.identityToString(ident) + "\"");
	}
	
	//
	// Evict as many elements as necessary.
	//
	evict();
    }

    synchronized public void
    destroyObject(Ice.Identity ident)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	//
	// Delete the Ice object from the database.
	//
        _dict.fastRemove(ident);

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
                _db.getCommunicator().getLogger().trace("Evictor", "destroyed \"" +
                                                        Ice.Util.identityToString(ident) + "\"");
            }
	}
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
		_db.getCommunicator().getLogger().trace("Evictor",
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
		    "Evictor",
		    "couldn't find \"" + Ice.Util.identityToString(ident) + "\" in the queue\n"
		    + "loading \"" + Ice.Util.identityToString(ident) + "\" from the database");
	    }
	    
	    //
	    // Load the Ice object from the database and add a
	    // servant for it.
	    //
	    Ice.Object servant = (Ice.Object)_dict.get(ident);
	    if(servant == null)
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
		_initializer.initialize(current.adapter, ident, servant);
	    }

	    //
	    // Add the new servant to the evictor queue.
	    //
	    element = add(ident, servant);
	}
	
	//
	// Increase the usage count of the evictor queue element.
	//
	++element.usageCount;
        if(current.mode != Ice.OperationMode.Nonmutating)
        {
            ++element.mutatingCount;
        }

	//
	// Evict as many elements as necessary.
	//
	evict();
	
	//
	// Set the cookie and return the servant for the Ice object.
	//
	cookie.value = (Ice.LocalObject)element;
	return element.servant;
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
        if(current.mode != Ice.OperationMode.Nonmutating)
        {
            assert(element.mutatingCount >= 1);
            --element.mutatingCount;
        }
	
        //
        // If the object has not been destroyed, notify the persistence
        // strategy about the operation.
        //
        if(!element.destroyed)
        {
            _strategy.invokedObject(this, current.id, servant, current.mode != Ice.OperationMode.Nonmutating,
                                    element.mutatingCount == 0, element.strategyCookie);
        }
	
	//
	// Evict as many elements as necessary.
	//
	evict();
    }

    synchronized public void
    deactivate()
    {
	if(!_deactivated)
	{
	    _deactivated = true;
	    
	    if(_trace >= 1)
	    {
		_db.getCommunicator().getLogger().trace("Evictor",
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
        // NOTE: Do not synchronize on the evictor mutex or else
        // deadlocks may occur.
        //
        _dict.fastPut(ident, servant);
    }

    EvictorI(DB db, PersistenceStrategy strategy)
    {
	_db = db;
	_dict = new IdentityObjectDict(db);
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
                assert(element.mutatingCount == 0);

                //
                // Notify the persistence strategy about the evicted object.
                //
                _strategy.evictedObject(this, ident, element.servant, element.strategyCookie);

		//
		// Remove element from the evictor queue.
		//
		p.remove();
		_evictorMap.remove(ident);

		if(_trace >= 2)
		{
		    _db.getCommunicator().getLogger().trace(
			"Evictor", 
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
    add(Ice.Identity ident, Ice.Object servant)
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
	element.servant = servant;
	element.position = _evictorList.iterator();
	element.usageCount = 0;    
	element.mutatingCount = 0;    
	element.destroyed = false;    
        element.strategyCookie = _strategy.activatedObject(ident, servant);

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
	Ice.Object servant;
	java.util.Iterator position;
	int usageCount;
	int mutatingCount;
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
    private LinkedList _evictorList = new LinkedList();

    private int _evictorSize = 10;
    private boolean _deactivated = false;
    private IdentityObjectDict _dict;

    private DB _db;
    private PersistenceStrategy _strategy;
    private ServantInitializer _initializer;
    private int _trace = 0;
}
