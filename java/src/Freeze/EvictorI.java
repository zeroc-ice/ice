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
    public EvictorI(Ice.Communicator communicator, String envName,
		    String dbName, boolean createDb)
    {
	_communicator = communicator;
	_dbEnvHolder = SharedDbEnv.get(communicator, envName);
	_dbEnv = _dbEnvHolder;

	init(envName, dbName, createDb);
    }

    public EvictorI(Ice.Communicator communicator, com.sleepycat.db.DbEnv dbEnv,
		    String dbName, boolean createDb)
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

    synchronized public void
    createObject(Ice.Identity ident, Ice.Object servant)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	//
	// Make a copy of ident in case the user later changes it
	// (used when inserting into list or map)
	//
	Ice.Identity identCopy = new Ice.Identity();
	identCopy.name = ident.name;
	identCopy.category = ident.category;
	
	EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	if(element != null)
	{ 
	    synchronized(element)
	    {
		switch(element.status)
		{
		    case clean:
		    {
			element.status = modified;
			addToModifiedQueue(identCopy, element);
			break;
		    }
		    case created:
		    case modified:
		    {
			//
			// Nothing to do.
			// No need to push it on the modified queue as a created resp
			// modified element is either already on the queue or about 
			// to be saved. When saved, it becomes clean.
			//
			break;
		    }  
		    case destroyed:
		    {
			element.status = modified;
			//
			// No need to push it on the modified queue, as a destroyed element
			// is either already on the queue or about to be saved. When saved,
			// it becomes dead.
			//
			break;
		    }
		    case dead:
		    {
			element.status = created;
			addToModifiedQueue(identCopy, element);
			break;
		    }
		    default:
		    {
			assert(false);
			break;
		    }
		}
		element.rec.servant = servant;
	    }

	    element.position.remove();
	    _evictorList.addFirst(identCopy);
	    element.position = _evictorList.iterator();
	}
	else
	{
	    //
	    // Create a new object
	    //
	    
	    ObjectRecord rec = new ObjectRecord();
	    rec.servant = servant;
	    rec.stats = new Statistics();
	    rec.stats.creationTime = System.currentTimeMillis();
	    rec.stats.lastSaveTime = 0;
	    rec.stats.avgSaveTime = 0;
	    
	    //
	    // Add an Ice object with its servant to the evictor queue.
	    //
	    element = new EvictorElement();
	    element.rec = rec;
	    element.usageCount = 0;    
	    element.status = created;
	    
	    _evictorMap.put(identCopy, element);
	    _evictorList.addFirst(identCopy);
	    
	    element.position = _evictorList.iterator();	
	    //
	    // Position the iterator "on" the element.
	    //
	    element.position.next();
	    
	    addToModifiedQueue(identCopy, element);
	    
	    //
	    // Evict as many elements as necessary.
	    //
	    evict();
	}
	
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor",
					    "created \"" + Ice.Util.identityToString(ident) + "\"");
	}
    }

    synchronized public void
    destroyObject(Ice.Identity ident)
    {
	if(_deactivated)
	{
	    throw new EvictorDeactivatedException();
	}

	EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	if(element != null)
	{
	    synchronized(element)
	    {
		switch(element.status)
		{
		    case clean:
		    {
			element.status = destroyed;

			//
			// Make a copy of ident in case the user later changes it
			// (used when inserting into list or map)
			//
			Ice.Identity identCopy = new Ice.Identity();
			identCopy.name = ident.name;
			identCopy.category = ident.category;
			
			addToModifiedQueue(identCopy, element);
			break;
		    }
		    case created:
		    {
			element.status = dead;
			break;
		    }
		    case modified:
		    {
			element.status = destroyed;
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
	    }
	}
	else
	{
	    //
	    // Set a real ObjectRecord in case this object gets recreated
	    //

	    ObjectRecord rec = new ObjectRecord();
	    rec.servant = null;
	    rec.stats = new Statistics();
	    rec.stats.creationTime = System.currentTimeMillis();
	    rec.stats.lastSaveTime = 0;
	    rec.stats.avgSaveTime = 0;

	    //
	    // Add an Ice object with its servant to the evictor queue.
	    //
	    element = new EvictorElement();
	    element.rec = rec;
	    element.usageCount = 0;    
	    element.status = destroyed;
	    
	 
	    //
	    // Make a copy of ident in case the user later changes it
	    // (used when inserting into list or map)
	    //
	    Ice.Identity identCopy = new Ice.Identity();
	    identCopy.name = ident.name;
	    identCopy.category = ident.category;
	   
	    _evictorMap.put(identCopy, element);
	    _evictorList.addFirst(identCopy);
	    
	    element.position = _evictorList.iterator();	
	    //
	    // Position the iterator "on" the element.
	    //
	    element.position.next();
	    
	    addToModifiedQueue(identCopy, element);
	    
	    //
	    // Evict as many elements as necessary.
	    //
	    evict();
	}
	
	if(_trace >= 1)
	{
	    _communicator.getLogger().trace("Freeze::Evictor", "destroyed \"" +
					    Ice.Util.identityToString(ident) + "\"");
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

    synchronized public boolean
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
		synchronized(element)
		{
		    return (element.status != destroyed && element.status != dead);
		}
	    }
	}
	
	return dbHasObject(ident);
    }

    public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
	//
	// Need to copy current.id, as Ice caches and reuses it
	//
	Ice.Identity ident = new Ice.Identity();
	ident.name = current.id.name;
	ident.category = current.id.category;

	ObjectRecord rec = null;
   
	for(;;)
	{
	    EvictorElement element;

	    synchronized(this)
	    {
		// 
		// If this operation is called on a deactivated servant locator,
		// it's a bug in Ice.
		//
		assert(!_deactivated);

		element = (EvictorElement)_evictorMap.get(ident);

		if(element != null)
		{
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

		    element.usageCount++;
		    cookie.value = (Ice.LocalObject)element;
		    
		    //
		    // Later (after releasing the mutex), check that this
		    // object is not dead or destroyed
		    //
		}
		else if(rec != null)
		{
		    //
		    // Proceed with the object loaded in the previous loop
		    //
		    
		    //
		    // If an initializer is installed, call it now.
		    //
		    if(_initializer != null)
		    {
			_initializer.initialize(current.adapter, ident, rec.servant);
		    }
		    
		    //
		    // Add an Ice object with its servant to the evictor queue.
		    //
		    
		    element = new EvictorElement();
		    element.rec = rec;
		    element.usageCount = 1;    
		    element.status = clean;

		    _evictorMap.put(ident, element);
		    _evictorList.addFirst(ident);
       
		    element.position = _evictorList.iterator();	
		    //
		    // Position the iterator "on" the element.
		    //
		    element.position.next();
		    
		    cookie.value = (Ice.LocalObject)element;
		    
		    //
		    // Evict as many elements as necessary.
		    //
		    evict();
		    
		    return rec.servant;
		}
		//
		// Else fall to the after-sync processing
		//
	    }
	
	    if(element != null)
	    {
		if(_trace >= 2)
		{
		    _communicator.getLogger().trace("Freeze::Evictor",
						    "found \"" + Ice.Util.identityToString(ident) +
						    "\" in the queue");
		}
		
		//
		// Return servant if object not dead or destroyed
		//
		synchronized(element)
		{
		    if(element.status != destroyed && element.status != dead)
		    {
			return element.rec.servant;
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
		
		rec = getObject(ident);
		
		if(rec == null)
		{
		    //
		    // The Ice object with the given identity does not exist,
		    // client will get an ObjectNotExistException.
		    //
		    return null;
		}
		//
		// Else loop
		//
	    }
	}
    }


    public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
	assert(servant != null);

	EvictorElement element = (EvictorElement)cookie;
	boolean enqueue = false;

	if(current.mode != Ice.OperationMode.Nonmutating)
	{
	    synchronized(element)
	    {
		if(element.status == clean)
		{
		    //
		    // Assume this operation updated the object
		    // 
		    element.status = modified;
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
	    assert(element.usageCount >= 1);
	    element.usageCount--;
	    
	    if(enqueue)
	    {
		//
		// Need to copy current.id, as Ice caches and reuses it
		//
		Ice.Identity ident = new Ice.Identity();
		ident.name = current.id.name;
		ident.category = current.id.category;
		
		addToModifiedQueue(ident, element);
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
    
	    //
	    // Usage count release
	    //
	    java.util.List releaseAfterStreaming = new java.util.ArrayList();
	    java.util.List releaseAfterCommit = new java.util.ArrayList();
	    
	    java.util.List streamedObjectQueue = new java.util.ArrayList();
	    
	    long saveStart = System.currentTimeMillis();
	    
	    //
	    // Stream each element
	    //
	    for(int i = 0; i < size; i++)
	    {
		EvictorElement element = (EvictorElement)_evictorMap.get((Ice.Identity)allObjects.get(i));
		
		synchronized(element)
		{
		    ObjectRecord rec = element.rec;
		    
		    boolean streamIt = true;
		    byte status = element.status;

		    switch(status)
		    {
			case created:
			{
			    element.status = clean;
			    releaseAfterCommit.add(element);
			    break;
			}   
			case modified:
			{
			    element.status = clean;
			    releaseAfterStreaming.add(element);
			    break;
			}
			case destroyed:
			{
			    element.status = dead;
			    releaseAfterCommit.add(element);
			    break;
			}   
			default:
			{
			    //
			    // Nothing to do (could be a duplicate)
			    //
			    streamIt = false;
			    releaseAfterStreaming.add(element);
			    break;
			}
		    }
		    
		    if(streamIt)
		    {
			int index = streamedObjectQueue.size();
			StreamedObject obj = new StreamedObject();
			streamedObjectQueue.add(obj);
			
			obj.key = IdentityObjectRecordDict.encodeKeyImpl(allObjects.get(i), _communicator);
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
	
	    allObjects.clear();
	
	    if(releaseAfterStreaming.size() > 0)
	    {
		synchronized(this)
		{
		    for(int i = 0; i < releaseAfterStreaming.size(); i++)
		    {    
			EvictorElement element = (EvictorElement)releaseAfterStreaming.get(i);
			element.usageCount--;
		    }
		}
		releaseAfterStreaming.clear();
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
				
				if(obj.status == destroyed)
				{
				    //
				    // May not exist in the database
				    //
				    com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(obj.key);
				    
				    int err = _db.del(tx, dbKey, 0);
				    if(err != 0 && err != com.sleepycat.db.Db.DB_NOTFOUND)
				    {
					//
					// Bug in Freeze
					//
					throw new DBException();
				    }	
				}
				else
				{
				    //
				    // We can't use NOOVERWRITE as some 'created' objects may
				    // actually be already in the database
				    //
				    
				    com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(obj.key);
				    com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(obj.value);
				    
				    int err = _db.put(tx, dbKey, dbValue, 0);
				    if(err != 0)
				    {
					//
					// Bug in Freeze
					//
					throw new DBException();
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
		
		for(int i = 0; i < releaseAfterCommit.size(); i++)
		{    
		    EvictorElement element = (EvictorElement)releaseAfterCommit.get(i);
		    element.usageCount--;
		}
		releaseAfterCommit.clear();
		
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

    private void
    init(String envName, String dbName, boolean createDb)
    {
	_trace = _communicator.getProperties().getPropertyAsInt(
	    "Freeze.Trace.Evictor");

	_errorPrefix = "Freeze Evictor DbEnv(\"" + envName + "\") Db(\"" +
	    dbName + "\") :";

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
	    _db.open(null, dbName, null, com.sleepycat.db.Db.DB_BTREE, 
		     flags, 0);

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
	for(;;)
	{
	    try
	    {
		byte[] key = IdentityObjectRecordDict.encodeKeyImpl(ident, _communicator);
		com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
		
		//
		// Keep 0 length since we're not interested in the data
		//
		com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
		dbValue.set_flags(com.sleepycat.db.Db.DB_DBT_USERMEM | 
				  com.sleepycat.db.Db.DB_DBT_PARTIAL);
		
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

    private ObjectRecord
    getObject(Ice.Identity ident)
    {
	byte[] key = IdentityObjectRecordDict.encodeKeyImpl(ident, _communicator);
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
	
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();

	for(;;)
	{
	    try
	    {
		int err = _db.get(null, dbKey, dbValue, 0);
		
		if(err == 0)
		{
		    return (ObjectRecord) IdentityObjectRecordDict.decodeValueImpl(
			dbValue.get_data(), _communicator);
		}
		else if(err == com.sleepycat.db.Db.DB_NOTFOUND)
		{
		    return null;
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
    addToModifiedQueue(Ice.Identity ident, EvictorElement element)
    {
	element.usageCount++;
	_modifiedQueue.add(ident);
	
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
	return IdentityObjectRecordDict.encodeValueImpl(rec, _communicator);
    }

    class StreamedObject
    {
	byte[] key;
	byte[] value;
	byte status;
    };


    class EvictorElement extends Ice.LocalObjectImpl
    {
	byte status;
        ObjectRecord rec;
	java.util.Iterator position;
	int usageCount;
    };
    
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
    // The _modifiedQueue contains a queue of all modified objects
    // Each element in the queue "owns" a usage count, to ensure the
    // pointed element remains in the map.
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
}
