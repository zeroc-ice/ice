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

class EvictorI extends Ice.LocalObjectImpl implements Evictor, Runnable
{
    public
    EvictorI(Ice.Communicator communicator, String envName, String dbName, boolean createDb)
    {
	_communicator = communicator;
	_dbEnvHolder = SharedDbEnv.get(communicator, envName);
	_dbEnv = _dbEnvHolder;

	init(envName, dbName, createDb);
    }

    public
    EvictorI(Ice.Communicator communicator, com.sleepycat.db.DbEnv dbEnv, String dbName, boolean createDb)
    {
	_communicator = communicator;
	_dbEnvHolder = null;
	_dbEnv = dbEnv;

	init("External", dbName, createDb);
    }
   
    protected void
    finalize()
    {
        if(!_deactivated)
        {
            _communicator.getLogger().warning("evictor has not been deactivated");
        }
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
    saveNow()
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}
	
	saveNowNoSync();
    }

    public void
    createObject(Ice.Identity ident, Ice.Object servant)
    {
	EvictorElement loadedElement = null;
	int loadedElementGeneration = 0;
	boolean triedToLoadElement = false;

	//
	// Make a copy of ident in case the user later changes it
	// (used when inserting into list or map)
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;

	for(;;)
	{
	    synchronized(this)
	    {
		if(_deactivated)
		{
		    throw new EvictorDeactivatedException();
		}

		EvictorElement element = (EvictorElement)_evictorMap.get(ident);

		if(element == null && triedToLoadElement)
		{
		    if(loadedElementGeneration == _generation)
		    {
			if(loadedElement != null)
			{
			    element = insertElement(null, identCopy, loadedElement);
			}
		    }
		    else
		    {
			loadedElement = null;
			triedToLoadElement = false;
		    }
		}

		boolean replacing = (element != null);

		if(replacing || triedToLoadElement)
		{
		    if(replacing)
		    {
			//
			// Destroy all existing facets
			//
			
			java.util.Iterator p = element.facets.entrySet().iterator();

			while(p.hasNext())
			{
			    java.util.Map.Entry entry = (java.util.Map.Entry) p.next();
			    destroyFacetImpl((Facet) entry.getValue());
			}
		    }
		    else
		    {
			//
			// Let's insert an empty EvictorElement
			//
			element = new EvictorElement();
			_evictorMap.put(identCopy, element);
			_evictorList.addFirst(identCopy);
			element.identity = identCopy;
			element.position = _evictorList.iterator();
			element.position.next();
		    }

		    //
		    // Add all the new facets (recursively)
		    //
		    addFacetImpl(element, servant, new String[0], replacing);

		    //
		    // Evict as many elements as necessary
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
	    assert(loadedElement == null);
	    assert(triedToLoadElement == false);
	    loadedElement = load(ident);
	    triedToLoadElement = true;
	}

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor",
					    "created \"" + Ice.Util.identityToString(ident) + "\"");
	}
    }

    public void
    addFacet(Ice.Identity ident, String[] facetPath, Ice.Object servant)
    {
	if(facetPath.length == 0)
	{
	    throw new EmptyFacetPathException();
	}
	
	EvictorElement loadedElement = null;
	int loadedElementGeneration = 0;

	//
	// Make a copy of ident in case the user later changes it
	// (used when inserting into list or map)
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;

	for(;;)
	{
	    synchronized(this)
	    {
		if(_deactivated)
		{
		    throw new EvictorDeactivatedException();
		}

		EvictorElement element = (EvictorElement)_evictorMap.get(ident);

		if(element == null && loadedElement != null)
		{
		    if(loadedElementGeneration == _generation)
		    {
			element = insertElement(null, identCopy, loadedElement);
		    }
		    else
		    {
			//
			// Discard loadedElement
			//
			loadedElement = null;
		    }
		}

		if(element != null)
		{
		    String[] parentPath = new String[facetPath.length - 1];
		    System.arraycopy(facetPath, 0, parentPath, 0, facetPath.length - 1);
		    
		    Facet facet = (Facet) element.facets.get(new StringArray(parentPath));
		    if(facet == null)
		    {
			throw new Ice.FacetNotExistException();
		    }
		    
		    synchronized(facet)
		    {
			if(facet.status == dead || facet.status == destroyed)
			{
			    throw new Ice.FacetNotExistException();
			}
			
			//
			// Throws AlreadyRegisterException if the facet is already registered
			//
			facet.rec.servant.ice_addFacet(servant, facetPath[facetPath.length - 1]);
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

	    assert(loadedElement == null);
	    
	    //
	    // Load object and loop
	    //
	    loadedElement = load(ident);
	    if(loadedElement == null)
	    {
		throw new Ice.ObjectNotExistException();
	    }
	}
		    
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor",
					    "added facet to \"" + Ice.Util.identityToString(ident) + "\"");
	}
    }

    public void
    destroyObject(Ice.Identity ident)
    {
	EvictorElement loadedElement = null;
	int loadedElementGeneration = 0;
	boolean triedToLoadElement = false;

	//
	// Make a copy of ident in case the user later changes it
	// (used when inserting into list or map)
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;

	for(;;)
	{
	    synchronized(this)
	    {
		if(_deactivated)
		{
		    throw new EvictorDeactivatedException();
		}

		EvictorElement element = (EvictorElement)_evictorMap.get(ident);

		if(element == null && triedToLoadElement)
		{
		    if(loadedElementGeneration == _generation)
		    {
			if(loadedElement != null)
			{
			    element = insertElement(null, identCopy, loadedElement);
			}
		    }
		    else
		    {
			loadedElement = null;
			triedToLoadElement = false;
		    }
		}

		boolean destroying = (element != null);

		if(destroying || triedToLoadElement)
		{
		    if(destroying)
		    {
			//
			// Destroy all existing facets
			//
			java.util.Iterator p = element.facets.entrySet().iterator();
			while(p.hasNext())
			{
			    java.util.Map.Entry entry = (java.util.Map.Entry) p.next();
			    destroyFacetImpl((Facet) entry.getValue());
			}
		    }
		    
		    //
		    // Evict as many elements as necessary
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
	    assert(loadedElement == null);
	    assert(triedToLoadElement == false);
	    loadedElement = load(ident);
	    triedToLoadElement = true;
	}

	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor",
					    "destroyed \"" + Ice.Util.identityToString(ident) + "\"");
	}
    }

    public void
    removeFacet(Ice.Identity ident, String facetPath[])
    {
	if(facetPath.length == 0)
	{
	    throw new EmptyFacetPathException();
	}
	
	EvictorElement loadedElement = null;
	int loadedElementGeneration = 0;

	//
	// Make a copy of ident in case the user later changes it
	// (used when inserting into list or map)
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;

	for(;;)
	{
	    synchronized(this)
	    {
		if(_deactivated)
		{
		    throw new EvictorDeactivatedException();
		}

		EvictorElement element = (EvictorElement)_evictorMap.get(ident);

		if(element == null && loadedElement != null)
		{
		    if(loadedElementGeneration == _generation)
		    {
			element = insertElement(null, identCopy, loadedElement);
		    }
		    else
		    {
			//
			// Discard loadedElement
			//
			loadedElement = null;
		    }
		}

		if(element != null)
		{
		    String[] parentPath = new String[facetPath.length - 1];
		    System.arraycopy(facetPath, 0, parentPath, 0, facetPath.length - 1);
		    
		    Facet facet = (Facet) element.facets.get(new StringArray(parentPath));
		    if(facet == null)
		    {
			throw new Ice.FacetNotExistException();
		    }
		    
		    synchronized(facet)
		    {
			if(facet.status == dead || facet.status == destroyed)
			{
			    throw new Ice.FacetNotExistException();
			}
			
			//
			// Throws NotRegisteredException if the facet is not registered
			//
			facet.rec.servant.ice_removeFacet(facetPath[facetPath.length - 1]);
		    }
		    removeFacetImpl(element.facets, facetPath);
		    evict();
		    break; // for(;;)
		}
		
		loadedElementGeneration = _generation;
	    }

	    assert(loadedElement == null);
	    
	    //
	    // Load object and loop
	    //
	    loadedElement = load(ident);
	    if(loadedElement == null)
	    {
		throw new Ice.ObjectNotExistException();
	    }
	}
		    
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor",
					    "removed facet from \"" + Ice.Util.identityToString(ident) + "\"");
	}
    }

    public void
    removeAllFacets(Ice.Identity ident)
    {
	EvictorElement loadedElement = null;
	int loadedElementGeneration = 0;

	//
	// Make a copy of ident in case the user later changes it
	// (used when inserting into list or map)
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;

	for(;;)
	{
	    synchronized(this)
	    {
		if(_deactivated)
		{
		    throw new EvictorDeactivatedException();
		}

		EvictorElement element = (EvictorElement)_evictorMap.get(ident);

		if(element == null && loadedElement != null)
		{
		    if(loadedElementGeneration == _generation)
		    {
			element = insertElement(null, identCopy, loadedElement);
		    }
		    else
		    {
			//
			// Discard loadedElement
			//
			loadedElement = null;
		    }
		}

		if(element != null)
		{
		    Facet facet =  element.mainObject;
		    synchronized(facet)
		    {
			if(facet.status == dead || facet.status == destroyed)
			{
			    throw new Ice.ObjectNotExistException();
			}
			facet.rec.servant.ice_removeAllFacets();
		    }
		    
		    //
		    // Destroy all facets except main object
		    //
		    java.util.Iterator p = element.facets.entrySet().iterator();
		    
		    while(p.hasNext())
		    {
			java.util.Map.Entry entry = (java.util.Map.Entry) p.next();
			if(entry.getValue() != element.mainObject)
			{
			    destroyFacetImpl((Facet) entry.getValue());
			}
		    }

		    evict();
		    break; // for(;;)
		}
		
		loadedElementGeneration = _generation;
	    }

	    assert(loadedElement == null);
	    
	    //
	    // Load object and loop
	    //
	    loadedElement = load(ident);
	    if(loadedElement == null)
	    {
		throw new Ice.ObjectNotExistException();
	    }
	}
		    
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor",
					    "removed all facets from \"" + Ice.Util.identityToString(ident) + "\"");
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

    public EvictorIterator
    getIterator()
    {
	synchronized(this)
	{
	    if(_deactivated)
	    {
		throw new EvictorDeactivatedException();
	    }
	    saveNowNoSync();
	}

	return new EvictorIteratorI(_db, _communicator, _errorPrefix);
    }

    public boolean
    hasObject(Ice.Identity ident)
    {
	synchronized(this)
	{
	    if(_deactivated)
	    {
		throw new EvictorDeactivatedException();
	    }

	    EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	    if(element != null)
	    {
		synchronized(element.mainObject)
		{
		    return (element.mainObject.status != destroyed && element.mainObject.status != dead);
		}
	    }
	}
	
	return dbHasObject(ident);
    }

    public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
	EvictorElement loadedElement = null;
	int loadedElementGeneration = 0;
	cookie.value = null;

	//
	// Need to copy current.id, as Ice caches and reuses it
	//
	Ice.Identity ident = new Ice.Identity();
	ident.name = current.id.name;
	ident.category = current.id.category;
   
	for(;;)
	{
	    EvictorElement element;
	    boolean objectFound = false;
	    boolean newObject = false;

	    synchronized(this)
	    {
		// 
		// If this operation is called on a deactivated servant locator,
		// it's a bug in Ice.
		//
		assert(!_deactivated);

		element = (EvictorElement)_evictorMap.get(ident);

		if(element == null && loadedElement != null)
		{
		    if(loadedElementGeneration == _generation)
		    {
			element = insertElement(null, ident, loadedElement);
			newObject = true;
		    }
		    else
		    {
			//
			// Discard loadedElement
			//
			loadedElement = null;
		    }
		}

		objectFound = (element != null);

		if(objectFound)
		{
		    //
		    // Ice object found in evictor map. Push it to the front of
		    // the evictor list, so that it will be evicted last.
		    //
		    if(!newObject)
		    {
			element.position.remove();
			_evictorList.addFirst(ident);
			element.position = _evictorList.iterator();
			//
			// Position the iterator "on" the element.
			//
			element.position.next();
		    }

		    element.usageCount++;

		    assert(current.facet != null);
		    Facet facet = (Facet) element.facets.get(new StringArray(current.facet));
		    if(facet != null)
		    {
			cookie.value = facet;
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
		if(_trace >= 2)
		{
		    _communicator.getLogger().trace("Freeze::Evictor",
						    "found \"" + Ice.Util.identityToString(ident) +
						    "\" in the queue");
		}
		
		if(cookie.value == null)
		{
		    Ice.Object result = null;
		    synchronized(element.mainObject)
		    {
			if(element.mainObject.status != destroyed && element.mainObject.status != dead)
			{
			    result = element.mainObject.rec.servant;
			}

		    }
		    if(_trace >= 2)
		    {
			_communicator.getLogger().trace("Freeze::Evictor",
							" \"" + Ice.Util.identityToString(ident) +
							"\" does not have the desired facet");
		    }
		    synchronized(this)
		    {
			element.usageCount--;
			return result;
		    }
		}
		else
		{
		    synchronized(element.mainObject)
		    {
			if(element.mainObject.status != destroyed && element.mainObject.status != dead)
			{
			    return element.mainObject.rec.servant;
			}
		    }
		}
	
		//
		// Object is destroyed or dead: clean-up
		//
		if(_trace >= 2)
		{
		    _communicator.getLogger().trace("Freeze::Evictor",
						    "\"" + Ice.Util.identityToString(ident) +
						    "\" was dead or destroyed");
		}
		synchronized(this)
		{
		    element.usageCount--;
		    return null;
		}
	    }
	    else
	    {
		//
		// Load object now and loop
		//
		
		if(_trace >= 2)
		{
		    _communicator.getLogger().trace(
			"Freeze::Evictor",
			"couldn't find \"" + Ice.Util.identityToString(ident) + "\" in the queue\n"
			+ "loading \"" + Ice.Util.identityToString(ident) + "\" from the database");
		}
		
		loadedElement = load(ident);
       	
		if(loadedElement == null)
		{
		    //
		    // The Ice object with the given identity does not exist,
		    // client will get an ObjectNotExistException.
		    //
		    return null;
		}
	    }
	}
    }

    public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
	assert(servant != null);

	if(cookie != null)
	{
	    Facet facet= (Facet)cookie;
	    assert(facet != null);

	    boolean enqueue = false;

	    if(current.mode != Ice.OperationMode.Nonmutating)
	    {
		synchronized(facet)
		{
		    if(facet.status == clean)
		    {
			//
			// Assume this operation updated the object
			// 
			facet.status = modified;
			enqueue = true;
		    }
		}
	    }
	
	    synchronized(this)
	    {
		assert(!_deactivated);
		
		//
		// Decrease the usage count of the evictor queue element.
		//
		assert(facet.element.usageCount >= 1);
		facet.element.usageCount--;
		
		if(enqueue)
		{
		    //
		    // Need to copy current.id, as Ice caches and reuses it
		    //
		    Ice.Identity ident = new Ice.Identity();
		    ident.name = current.id.name;
		    ident.category = current.id.category;
		    
		    addToModifiedQueue(facet);
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
    }

    public void
    deactivate(String category)
    {
	boolean joinAndClose = false;

	synchronized(this)
	{
	    if(!_deactivated)
	    {    
		if(_trace >= 1)
		{
		    _communicator.getLogger().trace(
			"Freeze::Evictor",
			"deactivating, saving unsaved Ice objects to the database");
		}
		
		saveNowNoSync();

		//
		// Set the evictor size to zero, meaning that we will evict
		// everything possible.
		//
		_evictorSize = 0;
		evict();

		_deactivated = true;
		notifyAll();
		joinAndClose = true;
	    }
	}
	
	if(joinAndClose)
	{
	    for(;;)
	    {
		try
		{
		    _thread.join();
		    break;
		}
		catch(InterruptedException ex)
		{
		}
	    }

	    try
	    {
		_db.close(0);
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DBException ex = new DBException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "Db.close: " + dx.getMessage();
		throw ex;
	    }
	    _db = null;
	    if(_dbEnvHolder != null)
	    {
		_dbEnvHolder.close();
		_dbEnvHolder = null;
	    }
	    _dbEnv = null;
	    _initializer = null;
	}
    }

    public void
    run()
    {
	for(;;)
	{
	    java.util.List allObjects;
	    int saveNowThreadsSize = 0;
	    
	    synchronized(this)
	    {
		while((!_deactivated) &&
		      (_saveNowThreads.size() == 0) &&
		      (_saveSizeTrigger < 0 || _modifiedQueue.size() < _saveSizeTrigger))
		{
		    try
		    {
			if(_savePeriod == 0)
			{
			    wait();
			}
			else
			{
			    long preSave = System.currentTimeMillis();
			    wait(_savePeriod);
			    if(System.currentTimeMillis() > preSave + _savePeriod)
			    {
				break;
			    }
			}
		    }
		    catch(InterruptedException ex)
		    {
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
		
		allObjects = _modifiedQueue;
		_modifiedQueue = new java.util.ArrayList();
	    }
	    
	    int size = allObjects.size();
        
	    java.util.List streamedObjectQueue = new java.util.ArrayList();
	    
	    long saveStart = System.currentTimeMillis();
	    
	    //
	    // Stream each element
	    //
	    for(int i = 0; i < size; i++)
	    {
		Facet facet = (Facet) allObjects.get(i);
		
		synchronized(facet)
		{
		    ObjectRecord rec = facet.rec;
		    
		    boolean streamIt = true;
		    byte status = facet.status;

		    switch(status)
		    {
			case created:
			case modified:
			{
			    facet.status = clean;
			    break;
			}   
			case destroyed:
			{
			    facet.status = dead;
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
			int index = streamedObjectQueue.size();
			StreamedObject obj = new StreamedObject();
			streamedObjectQueue.add(obj);
			
			EvictorStorageKey esk = new EvictorStorageKey();
			esk.identity = facet.element.identity;
			esk.facet = facet.path;
			obj.key = marshalKey(esk, _communicator);
			obj.status = status;
			if(status != destroyed)
			{
			    synchronized(rec.servant)
			    {
				obj.value = writeObjectRecordToValue(saveStart, rec);
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
	    int txSize = streamedObjectQueue.size();
	    boolean tryAgain;
	    
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
			com.sleepycat.db.DbTxn tx = _dbEnv.txn_begin(null, 0);
			try
			{   
			    for(int i = 0; i < txSize; i++)
			    {
				StreamedObject obj = (StreamedObject) streamedObjectQueue.get(i);
				
				switch(obj.status)
				{
				    case created:
				    case modified:
				    {
					com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(obj.key);
					com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(obj.value);
					
					int flags = (obj.status == created) ? com.sleepycat.db.Db.DB_NOOVERWRITE : 0;

					int err = _db.put(tx, dbKey, dbValue, flags);
					if(err != 0)
					{
					    throw new DBException();
					}
					break;
				    }
				    case destroyed:
				    {
					com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(obj.key);
					
					int err = _db.del(tx, dbKey, 0);
					if(err != 0)
					{
					    throw new DBException();
					}
					break;
				    }
				    default:
				    {
					assert(false);
				    }
				}
			    }
			}
			catch(com.sleepycat.db.DbException dx)
			{
			    tx.abort();
			    throw dx;
			}
			tx.commit(0);
			
			for(int i = 0; i < txSize; i++)
			{
			    streamedObjectQueue.remove(0);
			}
			
			if(_trace >= 2)
			{
			    long now = System.currentTimeMillis();
			    _communicator.getLogger().trace(
				"Freeze::Evictor",
				"saved " + txSize + " objects in " + (now - saveStart) + " ms");
			    saveStart = now;
			}
		    }
		    catch(com.sleepycat.db.DbDeadlockException deadlock)
		    {
			tryAgain = true;
			txSize = (txSize + 1)/2;
		    }
		    catch(com.sleepycat.db.DbException dx)
		    {
			DBException ex = new DBException();
			ex.initCause(dx);
			ex.message = _errorPrefix + "saving: " + dx.getMessage();
			throw ex;
		    }
		} 
	    } while(tryAgain);

	    synchronized(this)
	    {
		_generation++;
		
		for(int i = 0; i < allObjects.size(); i++)
		{    
		    Facet facet = (Facet) allObjects.get(i);
		    facet.element.usageCount--;
		}
		allObjects.clear();
		evict();

		if(saveNowThreadsSize > 0)
		{
		    for(int i = 0; i < saveNowThreadsSize; i++)
		    {
			_saveNowThreads.remove(0);
		    }
		    notifyAll();
		}
	    }
	    
	    _lastSave = System.currentTimeMillis();
	}
    }

    static byte[]
    marshalRootKey(EvictorStorageKey v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
        try
        {
            v.identity.__write(os);
            java.nio.ByteBuffer buf = os.prepareWrite();
            byte[] r = new byte[buf.limit()];
            buf.get(r);
            return r;
        }
        finally
        {
            os.destroy();
        }
    }

    static byte[]
    marshalKey(EvictorStorageKey v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
        try
        {
            v.__write(os);
            java.nio.ByteBuffer buf = os.prepareWrite();
            byte[] r = new byte[buf.limit()];
            buf.get(r);
            return r;
        }
        finally
        {
            os.destroy();
        }
    }

    static EvictorStorageKey
    unmarshalKey(byte[] b, Ice.Communicator communicator)
    {
        IceInternal.BasicStream is = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
        try
        {
            is.resize(b.length, true);
            java.nio.ByteBuffer buf = is.prepareRead();
            buf.position(0);
            buf.put(b);
            buf.position(0);
            EvictorStorageKey key = new EvictorStorageKey();
            key.__read(is);
            return key;
        }
        finally
        {
            is.destroy();
        }
    }

    static byte[]
    marshalValue(ObjectRecord v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
        os.marshalFacets(false);
        try
        {
            v.__write(os);
            os.writePendingObjects();
            java.nio.ByteBuffer buf = os.prepareWrite();
            byte[] r = new byte[buf.limit()];
            buf.get(r);
            return r;
        }
        finally
        {
            os.destroy();
        }
    }

    static ObjectRecord
    unmarshalValue(byte[] b, Ice.Communicator communicator)
    {
        IceInternal.BasicStream is = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
        try
        {
            is.resize(b.length, true);
            java.nio.ByteBuffer buf = is.prepareRead();
            buf.position(0);
            buf.put(b);
            buf.position(0);
            ObjectRecord key = new ObjectRecord();
            key.__read(is);
            is.readPendingObjects();
            return key;
        }
        finally
        {
            is.destroy();
        }
    }

    private void
    init(String envName, String dbName, boolean createDb)
    {
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Evictor");

	_errorPrefix = "Freeze Evictor DbEnv(\"" + envName + "\") Db(\"" + dbName + "\"): ";

	String propertyPrefix = "Freeze.Evictor." + envName + '.' + dbName; 
	
	// 
	// By default, we save every minute or when the size of the modified 
	// queue reaches 10.
	//

	_saveSizeTrigger = _communicator.getProperties().getPropertyAsIntWithDefault(
	    propertyPrefix + ".SaveSizeTrigger", 10);

	_savePeriod = _communicator.getProperties().getPropertyAsIntWithDefault(
	    propertyPrefix + ".SavePeriod", 60 * 1000);
	
	try
	{
	    int flags = com.sleepycat.db.Db.DB_AUTO_COMMIT;
	    if(createDb)
	    {
		flags |= com.sleepycat.db.Db.DB_CREATE;
	    }
	    
	    _db = new com.sleepycat.db.Db(_dbEnv, 0);
	    _db.open(null, dbName, null, com.sleepycat.db.Db.DB_BTREE, flags, 0);

	    //
	    // TODO: FREEZE_DB_MODE
	    //
	}
	catch(java.io.FileNotFoundException dx)
	{
	    DBNotFoundException ex = new DBNotFoundException();
	    ex.initCause(dx);
	    ex.message = _errorPrefix + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    DBException ex = new DBException();
	    ex.initCause(dx);
	    ex.message = _errorPrefix + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	
	_lastSave = System.currentTimeMillis();
	
	//
	// Start saving thread
	//
	_thread = new Thread(this);
	_thread.start();
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
		// Remove element from the evictor queue.
		//
		p.remove();
		_evictorMap.remove(ident);

		if(_trace >= 2)
		{
		    _communicator.getLogger().trace(
			"Freeze::Evictor", 
			"evicted \"" + Ice.Util.identityToString(ident) +
			"\" from the queue\n" + "number of elements in the queue: " +
			_evictorMap.size());
		}
	    }
	}
    }

    private boolean
    dbHasObject(Ice.Identity ident)
    {
	EvictorStorageKey esk = new EvictorStorageKey();
	esk.identity = ident;
	esk.facet = null;
	
	byte[] key = marshalKey(esk, _communicator);

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
		
	//
	// Keep 0 length since we're not interested in the data
	//
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
	dbValue.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	
	for(;;)
	{
	    try
	    {	
		int err = _db.get(null, dbKey, dbValue, 0);
		
		if(err == 0)
		{
		    return true;
		}
		else if(err == com.sleepycat.db.Db.DB_NOTFOUND)
		{
		    return false;
		}
		else
		{
		    assert(false);
		    throw new DBException();
		}
	    }
	    catch(com.sleepycat.db.DbDeadlockException deadlock)
	    {
		//
		// Ignored, try again
		//
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DBException ex = new DBException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "Db.get: " + dx.getMessage();
		throw ex;
	    }
	}
    }

    private void
    addToModifiedQueue(Facet facet)
    {
	facet.element.usageCount++;
	_modifiedQueue.add(facet);
	
	if(_saveSizeTrigger >= 0 && _modifiedQueue.size() >= _saveSizeTrigger)
	{
	    notifyAll();
	}
    }
    
    private void
    saveNowNoSync()
    {
	Thread myself = Thread.currentThread();

	_saveNowThreads.add(myself);
	notifyAll();
	do
	{
	    try
	    {
		wait();
	    }
	    catch(InterruptedException ex)
	    {
	    }
	} while(_saveNowThreads.contains(myself));
    }
    
    private byte[]
    writeObjectRecordToValue(long saveStart, ObjectRecord rec)
    {
	//
	// Update stats first
	//
	Statistics stats = rec.stats;
	long diff = saveStart - (stats.creationTime + stats.lastSaveTime);
	if(stats.lastSaveTime == 0)
	{
	    stats.lastSaveTime = diff;
	    stats.avgSaveTime = diff;
	}
	else
	{
	    stats.lastSaveTime = saveStart - stats.creationTime;
	    stats.avgSaveTime = (long)(stats.avgSaveTime * 0.95 + diff * 0.05);
	}
	return marshalValue(rec, _communicator);
    }

    private EvictorElement
    load(Ice.Identity ident)
    {
        //
        // This method attempts to restore an object and all of its facets from the database. It works by
        // iterating over the database keys that match the "root" key. The root key is the encoded portion
        // of the EvictorStorageKey struct that the object and its facets all have in common, namely the
        // identity.
        //
	EvictorStorageKey rootEsk = new EvictorStorageKey();
	rootEsk.identity = ident;
	rootEsk.facet = null;
	byte[] root = marshalRootKey(rootEsk, _communicator);
        byte[] mainKey = marshalKey(rootEsk, _communicator);

	com.sleepycat.db.Dbt dbKey;
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();

	EvictorElement result = null;
	for(;;)
	{
	    result = new EvictorElement();

	    com.sleepycat.db.Dbc dbc = null;

	    try
	    {
		dbc = _db.cursor(null, 0);

		//
		// Get first pair
		//	
                dbKey = new com.sleepycat.db.Dbt(mainKey);
		int rs = dbc.get(dbKey, dbValue, com.sleepycat.db.Db.DB_SET);
	
		if(rs != 0)
		{ 
		    if(_trace >= 2)
		    {
			_communicator.getLogger().trace(
			    "Freeze::Evictor", 
			    "could not find \"" + Ice.Util.identityToString(ident) +
			    "\" in the database");
		    }
		    return null;
		}

		do
		{
		    //
		    // Unmarshal key and data and insert it into result's facet map
		    //
		    EvictorStorageKey esk = unmarshalKey(dbKey.get_data(), _communicator);
		    
		    Facet facet = new Facet(result);
		    facet.status = clean;
		    facet.rec = unmarshalValue(dbValue.get_data(), _communicator);
		    facet.path = esk.facet;
		    assert(facet.path != null);
		    result.facets.put(new StringArray(esk.facet), facet);
		    if(esk.facet.length == 0)
		    {
			result.mainObject = facet;
		    }

		    //
		    // Next facet
		    //
		    rs = dbc.get(dbKey, dbValue, com.sleepycat.db.Db.DB_NEXT);    
		}
		while(rs == 0 && startWith(dbKey.get_data(), root));
		
		break; // for (;;)
	    }
	    catch(com.sleepycat.db.DbDeadlockException deadlock)
	    {
		//
		// Ignored, try again
		//
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DBException ex = new DBException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "Db.get: " + dx.getMessage();
		throw ex;
	    }
	    finally
	    {
		if(dbc != null)
		{
		    try
		    {
			dbc.close();
			dbc = null;
		    }
		    catch(com.sleepycat.db.DbException dx)
		    {
		    }
		}
	    }
	}

	//
	// Let's fix-up the facets tree in result
	//
	java.util.Iterator p = result.facets.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry) p.next();
	    
	    String[] path = ((StringArray) entry.getKey()).array;
	    
	    if(path.length > 0)
	    {
		String[] parent = new String[path.length - 1];
		System.arraycopy(path, 0, parent, 0, path.length - 1);
		Facet parentFacet = (Facet) result.facets.get(new StringArray(parent));
		if(parentFacet == null)
		{
		    //
		    // TODO: log warning for this orphan facet
		    // 
		    assert(false);
		}
		else
		{
		    Facet childFacet = (Facet) entry.getValue();
		    parentFacet.rec.servant.ice_addFacet(childFacet.rec.servant, path[path.length - 1]);
		}
	    }
	}

	return result;
    }
    
    private EvictorElement
    insertElement(Ice.ObjectAdapter adapter, Ice.Identity ident, EvictorElement element)
    {
	if(_initializer != null)
	{
	    _initializer.initialize(adapter, ident, element.mainObject.rec.servant);
	}
	
	_evictorMap.put(ident, element);
	_evictorList.addFirst(ident);
       
	element.position = _evictorList.iterator();	
	//
	// Position the iterator "on" the element.
	//
	element.position.next();
	
	element.identity = ident;
	return element;
    }

    private void
    addFacetImpl(EvictorElement element, Ice.Object servant, String[] facetPath, boolean replacing)
    {
	java.util.Map facets = element.facets;
	
	boolean insertIt = true;

	StringArray facetPathArray = new StringArray(facetPath);

	if(replacing)
	{
	    Facet facet = (Facet) facets.get(facetPathArray);
	    if(facet != null)
	    {
		synchronized(facet)
		{
		    switch(facet.status)
		    {
			case clean:
			{
			    facet.status = modified;
			    addToModifiedQueue(facet);
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
			    facet.status = modified;
			    //
			    // No need to push it on the modified queue, as a destroyed facet
			    // is either already on the queue or about to be saved. When saved,
			    // it becomes dead.
			    //
			    break;
			}
			case dead:
			{
			    facet.status = created;
			    addToModifiedQueue(facet);
			    break;			    
			}
			default:
			{
			    assert(false);
			    break;
			}
		    }
		    facet.rec.servant = servant;
		    insertIt = false;
		}
	    }
	}
	
	if(insertIt)
	{
	    Facet facet = new Facet(element);
	    facet.status = created;
	    facet.path = facetPath;

	    facet.rec = new ObjectRecord();
	    ObjectRecord rec = facet.rec;
	    rec.servant = servant;
	    rec.stats = new Statistics();
	    rec.stats.creationTime = System.currentTimeMillis();
	    rec.stats.lastSaveTime = 0;
	    rec.stats.avgSaveTime = 0;

	    facets.put(facetPathArray, facet);
	    if(facetPath.length == 0)
	    {
		element.mainObject = facet;
	    }
	    addToModifiedQueue(facet);  
	}

	if(servant != null)
	{
	    //
	    // Add servant's facets
	    //
	    String[] facetList = servant.ice_facets(null);
	    for(int i = 0; i < facetList.length; i++)
	    {
		String[] newFacetPath = new String[facetPath.length + 1];
		System.arraycopy(facetPath, 0, newFacetPath, 0, facetPath.length);
		String currentName = facetList[i];
		newFacetPath[newFacetPath.length - 1] = currentName;
		addFacetImpl(element, servant.ice_findFacet(currentName), newFacetPath, replacing);  
	    }
	}
    }

    private void
    removeFacetImpl(java.util.Map facets, String[] facetPath)
    {
	Facet facet = (Facet) facets.get(new StringArray(facetPath));
	Ice.Object servant = null;
	
	if(facet != null)
	{
	    servant = destroyFacetImpl(facet);
	}
	//
	// else should we raise an exception?
	//
	
	if(servant != null)
	{
	    //
	    // Remove servant's facets
	    //
	    String[] facetList = servant.ice_facets(null);
	    for(int i = 0; i < facetList.length; i++)
	    {
		String[] newFacetPath = new String[facetPath.length + 1];
		System.arraycopy(facetPath, 0, newFacetPath, 0, facetPath.length);
		String currentName = facetList[i];
		newFacetPath[newFacetPath.length - 1] = currentName;
		removeFacetImpl(facets, newFacetPath);  
	    }
	}
    }

    private Ice.Object
    destroyFacetImpl(Facet facet)
    {
	synchronized(facet)
	{
	    switch(facet.status)
	    {
		case clean:
		{
		    facet.status = destroyed;
		    addToModifiedQueue(facet);
		    break;
		}
		case created:
		{
		    facet.status = dead;
		    break;
		}
		case modified:
		{
		    facet.status = destroyed;
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
		    assert(false);
		    break;
		}
	    }
	    return facet.rec.servant;
	}
    }

    private boolean
    startWith(byte[] key, byte[] root)
    {
	if(key.length >= root.length)
	{
	    for(int i = 0; i < root.length; i++)
	    {
		if(key[i] != root[i])
		{
		    return false;
		}
	    }
	    return true;
	}
	else
	{
	    return false;
	}
    }
	
    class StreamedObject
    {
	byte[] key;
	byte[] value;
	byte status;
    }

    class EvictorElement
    {
	java.util.Iterator position;
	int usageCount = 0;
	java.util.Map facets = new java.util.HashMap();
	Ice.Identity identity;
	Facet mainObject;
    }
    
    class Facet extends Ice.LocalObjectImpl
    {
	Facet(EvictorElement evictorElement)
	{
	    element = evictorElement;
	}
	byte status;
	ObjectRecord rec;
	EvictorElement element;
	String[] path;
    }
    
    //
    // Wrapper to use a String[] as key of a HashMap.
    //
    class StringArray
    {
	StringArray(String[] a)
	{
	    assert(a != null);
	    array = a;
	}

	public boolean equals(java.lang.Object o)
	{
	    if(o instanceof StringArray)
	    {
		StringArray rhs = (StringArray) o;
		if(rhs.array.length == array.length)
		{
		    for(int i = 0; i < array.length; i++)
		    {
			if(!array[i].equals(rhs.array[i]))
			{
			    return false;
			}
		    }
		    return true;
		}
	    }
	    return false;
	}
	
	public int hashCode()
	{
	    int result = 0;
	    for(int i = 0; i < array.length; i++)
	    {
		result ^= array[i].hashCode();
	    }
	    return result;
	}
	
	String[] array;
    }

    //
    // Clean object; can become modified or destroyed
    //
    private static final byte clean = 0;

    //
    // New objects; can become clean, dead or destroyed
    //
    private static final byte created = 1;

    //
    // Modified object; can become clean or destroyed
    //
    private static final byte modified = 2;

    //
    // Being saved. Can become dead or created
    //
    private static final byte destroyed = 3;

    //
    // Exists only in the Evictor; for example the object was created
    // and later destroyed (without a save in between), or it was
    // destroyed on disk but is still in use. Can become created.
    //
    private static final byte dead = 4;

    //
    // Map of Ice.Identity to EvictorElement
    //
    private java.util.Map _evictorMap = new java.util.HashMap();
    private int _evictorSize = 10;

    //
    // The C++ Evictor uses std::list<EvictorMap::iterator> which allows
    // holding of iterators across list changes. Unfortunately, Java
    // iterators are invalidated as soon as the underlying collection
    // is changed, so it's not possible to use the same technique.
    //
    // This is a list of Ice.Identity.
    //
    private Freeze.LinkedList _evictorList = new Freeze.LinkedList();

    //
    // The _modifiedQueue contains a queue of all modified facets
    // Each element in the queue "owns" a usage count, to ensure the
    // elements containing them remain in the map.
    //
    private java.util.List _modifiedQueue = new java.util.ArrayList();
    
    private boolean _deactivated = false;
   
    private Ice.Communicator _communicator;
    private SharedDbEnv      _dbEnvHolder;
    private com.sleepycat.db.DbEnv _dbEnv;
    private com.sleepycat.db.Db _db;
    private ServantInitializer _initializer;
    private int _trace = 0;
    
    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    private java.util.List _saveNowThreads = new java.util.ArrayList();

    private int _saveSizeTrigger;
    private long _savePeriod;
    private long _lastSave;

    private Thread _thread;
    private String _errorPrefix;
    
    //
    // _generation is incremented after committing changes
    // to disk, when releasing the usage count of the element
    // that contains the created/modified/destroyed facets. 
    // Like the usage count, it is protected by the Evictor mutex.
    //
    // It is used to detect updates when loading an element and its
    // facets without holding the Evictor mutex. If the generation
    // is the same before the loading and later when the Evictor
    // mutex is locked again, and the map still does not contain 
    // this element, then the loaded value is current.
    //
    private int _generation = 0;
}
