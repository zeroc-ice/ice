// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class EvictorI extends Ice.LocalObjectImpl implements Evictor, Runnable
{

    final static String defaultDb = "$default";
    final static String indexPrefix = "$index:";

    //
    // The WatchDogThread is used by the saving thread to ensure the
    // streaming of some object does not take more than timeout ms.
    // We only measure the time necessary to acquire the lock on the
    // object (servant), not the streaming itself.
    //
    class WatchDogThread extends Thread
    {
	WatchDogThread(long timeout, String name)
	{
	    super(name);
	    _timeout = timeout;
	    assert timeout > 0;
	}

	public synchronized void run()
	{
	    while(!_done)
	    {
		long startTime = 0;

		//
		// Unfortunely wait can be waken up by a spurious wakeup,
		// so we cannot reliably tell when we are woken up by a timeout.
		//
		try
		{
		    if(_active)
		    {
			startTime = System.currentTimeMillis();
			wait(_timeout);
		    }
		    else
		    {
			wait();
		    }
		}
		catch(InterruptedException e)
		{
		    //
		    // Ignore
		    //
		}

		if(!_done && _active && startTime > 0)
		{
		    //
		    // Did we timeout?
		    //
		    if(System.currentTimeMillis() - startTime >= _timeout)
		    {
			_communicator.getLogger().error
			    (_errorPrefix + "Fatal error: streaming watch dog thread timed out.");

			Util.handleFatalError(EvictorI.this, _communicator, null);
		    }
		}
	    }
	}

	synchronized void activate()
	{
	    _active = true;
	    notify();
	}
	
	synchronized void deactivate()
	{
	    _active = false;
	    notify();
	}

	synchronized void terminate()
	{
	    _done = true;
	    notify();
	}

	private final long _timeout;
	private boolean _done = false;
	private boolean _active = false;
    }

    
    class DeactivateController
    {
	synchronized void lock()
	{
	    if(_deactivated || _deactivating)
	    {
		throw new EvictorDeactivatedException();
	    }
	    _guardCount++;
	}

	synchronized void unlock()
	{
	    _guardCount--;
	    if(_deactivating && _guardCount == 0)
	    {
		//
		// Notify all the threads -- although we only want to
		// reach the thread doing the deactivation.
		//
		notifyAll();
	    }
	}
	
	synchronized boolean deactivated()
	{
	    return _deactivated || _deactivating;
	}
	
	synchronized boolean deactivate()
	{   
	    if(_deactivated)
	    {
		return false;
	    }
	    
	    if(_deactivating)
	    {
		//
		// Wait for deactivated
		//
		while(!_deactivated)
		{
		    try
		    {
			wait();
		    }
		    catch(InterruptedException e)
		    {
			// Ignored
		    }
		}
		return false;
	    }
	    else
	    {
		_deactivating = true;
		while(_guardCount > 0)
		{
		     if(_trace >= 1)
		     {
			 _communicator.getLogger().trace(
			     "Freeze.Evictor",  "*** Waiting for " + _guardCount 
			     + " threads to complete before starting deactivation.");
		     }
		     
		     try
		     {
			 wait();
		     }
		     catch(InterruptedException e)
		     {
			 // Ignored
		     }
		 }

		 if(_trace >= 1)
		 {
		     _communicator.getLogger().trace("Freeze.Evictor", "Starting deactivation.");
		 }
		 return true;
	     }
	}

	synchronized void deactivationComplete()
	{
	    if(_trace >= 1)
	    {
		_communicator.getLogger().trace("Freeze.Evictor", "Deactivation complete.");
	    }

	    _deactivated = true;
	    _deactivating = false;
	    notifyAll();
	}

	private boolean _deactivating = false;
	private boolean _deactivated = false;
	private int _guardCount = 0;
    }
    
    

    public
    EvictorI(Ice.ObjectAdapter adapter, String envName, String filename,
	     ServantInitializer initializer,
	     Index[] indices, boolean createDb)
    {
	_adapter = adapter;
	_communicator = adapter.getCommunicator();
	_initializer = initializer;
	_filename = filename;
	_createDb = createDb;
	init(envName, null, indices);
    }

    public
    EvictorI(Ice.ObjectAdapter adapter, String envName, 
	     com.sleepycat.db.DbEnv dbEnv, String filename, 
	     ServantInitializer initializer,
	     Index[] indices, boolean createDb)
    {
	_adapter = adapter;
	_communicator = adapter.getCommunicator();
	_initializer = initializer;
	_filename = filename;
	_createDb = createDb;
	init(envName, dbEnv, indices);
    }

    private void
    init(String envName, com.sleepycat.db.DbEnv dbEnv, Index[] indices)
    {
	
	_dbEnv = SharedDbEnv.get(_communicator, envName, dbEnv);

	
	_trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Evictor");
	_txTrace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Transaction");
	_deadlockWarning = _communicator.getProperties().getPropertyAsInt("Freeze.Warn.Deadlocks") != 0;

	_errorPrefix = "Freeze Evictor DbEnv(\"" + envName + "\") Db(\"" + _filename + "\"): ";

	String propertyPrefix = "Freeze.Evictor." + envName + '.' + _filename; 
	
	// 
	// By default, we save every minute or when the size of the modified 
	// queue reaches 10.
	//

	_saveSizeTrigger = _communicator.getProperties().getPropertyAsIntWithDefault
	    (propertyPrefix + ".SaveSizeTrigger", 10);

	_savePeriod = _communicator.getProperties().getPropertyAsIntWithDefault
	    (propertyPrefix + ".SavePeriod", 60 * 1000);

	//
	// By default, we save at most 10 * SaveSizeTrigger objects per transaction
	//
	_maxTxSize = _communicator.getProperties().getPropertyAsIntWithDefault
	    (propertyPrefix + ".MaxTxSize", 10 * _saveSizeTrigger);

	if(_maxTxSize <= 0)
	{
	    _maxTxSize = 100;
	}
	
	boolean populateEmptyIndices = (_communicator.getProperties().getPropertyAsIntWithDefault
					(propertyPrefix + ".PopulateEmptyIndices", 0) != 0);
	
	//
	// Instantiate all Dbs in 2 steps:
	// (1) iterate over the indices and create ObjectStore with indices
	// (2) open ObjectStores without indices
	//

	java.util.List dbs = allDbs();
	//
	// Add default db in case it's not there
	//
	dbs.add(defaultDb);

	
	if(indices != null)
	{
	    for(int i = 0; i < indices.length; ++i)
	    {
		String facet = indices[i].facet();
		
		if(_storeMap.get(facet) == null)
		{
		    java.util.List storeIndices = new java.util.LinkedList();
		    for(int j = i; j < indices.length; ++j)
		    {
			if(indices[j].facet().equals(facet))
			{
			    storeIndices.add(indices[j]);
			}
		    }
		    
		    ObjectStore store = new ObjectStore(facet, _createDb, this, storeIndices,
							populateEmptyIndices);
		    _storeMap.put(facet, store);
		}
	    }
	}

	java.util.Iterator p = dbs.iterator();
	while(p.hasNext())
	{
	    String facet = (String) p.next();
	    if(facet.equals(defaultDb))
	    {
		facet = "";
	    }
	    
	    if(_storeMap.get(facet) == null)
	    {
		ObjectStore store = new ObjectStore(facet, _createDb, this, new java.util.LinkedList(),
						    populateEmptyIndices);
		
		_storeMap.put(facet, store);
	    }
	}

	//
	// Start threads
	//
	String savingThreadName;
	
	String programName = _communicator.getProperties().getProperty("Ice.ProgramName");
        if(programName.length() > 0)
        {
            savingThreadName = programName + "-";
        }
	else
	{
	    savingThreadName = "";
	}
	String watchDogThreadName = savingThreadName + "FreezeEvictorWatchDogThread(" + envName + '.' + _filename + ")";
	savingThreadName += "FreezeEvictorThread(" + envName + '.' + _filename + ")";

	//
	// By default, no stream timeout
	//
	long streamTimeout =  _communicator.getProperties()
	    .getPropertyAsIntWithDefault(propertyPrefix + ".StreamTimeout", 0) * 1000;

	if(streamTimeout > 0)
	{
	    _watchDogThread = new WatchDogThread(streamTimeout, watchDogThreadName);
	    _watchDogThread.start();
	}

	_thread = new Thread(this, savingThreadName);
	_thread.start();
    }

   
    protected void
    finalize()
    {
        if(!_deactivateController.deactivated())
        {
            _communicator.getLogger().warning("evictor has not been deactivated");
	    deactivate("");
        }
    }

    synchronized public void
    setSize(int evictorSize)
    {
	_deactivateController.lock();

	try
	{
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
	finally
	{
	    _deactivateController.unlock();
	}
    }
	
    synchronized public int
    getSize()
    {	
	return _evictorSize;
    }
    
    public Ice.ObjectPrx
    add(Ice.Object servant, Ice.Identity ident)
    {
	return addFacet(servant, ident, "");
    }

    
    public Ice.ObjectPrx
    addFacet(Ice.Object servant, Ice.Identity ident, String facet)
    {
	checkIdentity(ident);
       
	if(facet == null)
        {
            facet = "";
        }

	//
	// Need to clone in case the given ident changes.
	//
	try
	{
	    ident = (Ice.Identity) ident.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	    assert false;
	}

	_deactivateController.lock();

	try
	{
	    ObjectStore store = null;
	    
	    for(;;)
	    {
		synchronized(this)
		{ 
		    Object o = _storeMap.get(facet);
		    
		    if(o == null)
		    {
			if(store != null)
			{
			    _storeMap.put(facet, store);
			}
		    }
		    else
		    {
			if(store != null)
			{
			    store.close();
			}
			store = (ObjectStore) o;
		    }
		}
		
		if(store == null)
		{
		    assert facet.length() > 0;
		    store = new ObjectStore(facet, _createDb, this, new java.util.LinkedList(), false);
		    // loop
		}
		else
		{
		    break; // for(;;)
		}
	    }
	    
	    assert store != null;
	    boolean alreadyThere = false;
	    
	    
	    for(;;)
	    {
		//
		// Create a new entry
		//
		
		EvictorElement element = new EvictorElement(ident, store);
		element.status = EvictorElement.dead;
		element.rec = new ObjectRecord();
		element.rec.stats = new Statistics();
		
		Object o = store.cache().putIfAbsent(ident, element);
		
		if(o != null)
		{
		    element = (EvictorElement) o;
		}
		
		synchronized(this)
		{	    
		    if(element.stale)
		    {
			//
			// Try again
			// 
			continue;
		    }
		    fixEvictPosition(element);
		    
		    synchronized(element)
		    {
			switch(element.status)
			{
			    case EvictorElement.clean:
			    case EvictorElement.created:
			    case EvictorElement.modified:
			    {
				alreadyThere = true;
				break;
			    }  
			    case EvictorElement.destroyed:
			    {
				element.status = EvictorElement.modified;
				element.rec.servant = servant;
				
				//
				// No need to push it on the modified queue, as a destroyed object
				// is either already on the queue or about to be saved. When saved,
				// it becomes dead.
				//
				break;
			    }
			    case EvictorElement.dead:
			    {
				element.status = EvictorElement.created;
				ObjectRecord rec = element.rec;
				
				rec.servant = servant;
				rec.stats.creationTime = System.currentTimeMillis();
				rec.stats.lastSaveTime = 0;
				rec.stats.avgSaveTime = 0;
				
				addToModifiedQueue(element);
				break;
			    }
			    default:
			    {
				assert false;
				break;
			    }
			}
		    }
		}
		break; // for(;;)
	    }
	    
	    if(alreadyThere)
	    {
		Ice.AlreadyRegisteredException ex = new Ice.AlreadyRegisteredException();
		ex.kindOfObject = "servant";
		ex.id = Ice.Util.identityToString(ident);
		if(facet.length() > 0)
		{
		    ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
		}
		throw ex;
	    }
	    
	    if(_trace >= 1)
	    {
		String objString = "object \"" + Ice.Util.identityToString(ident) + "\"";
		if(!facet.equals(""))
		{
		    objString += " with facet \"" + facet + "\"";
		}
		
		_communicator.getLogger().trace(
		    "Freeze.Evictor", 
		    "added " + objString + " in the database");
	    }
	    
	    Ice.ObjectPrx obj = _adapter.createProxy(ident);
	    if(facet.length() > 0)
	    {
		obj = obj.ice_newFacet(facet);
	    }
	    return obj;
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }


    //
    // Deprecated
    //
    public void
    createObject(Ice.Identity ident, Ice.Object servant)
    {
	checkIdentity(ident);

	//
	// Need to clone in case the given ident changes.
	//
	try
	{
	    ident = (Ice.Identity) ident.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	    assert false;
	}

	_deactivateController.lock();
	
	try
	{
	    ObjectStore store = findStore("");
	    assert store != null;
	    
	    for(;;)
	    {
		//
		// Create a new entry
		//
		
		EvictorElement element = new EvictorElement(ident, store);
		element.status = EvictorElement.dead;
		element.rec = new ObjectRecord();
		element.rec.stats = new Statistics();
		
		Object o = store.cache().putIfAbsent(ident, element);
		
		if(o != null)
		{
		    element = (EvictorElement) o;
		}
		
		synchronized(this)
		{
		    if(element.stale)
		    {
			//
			// Try again
			// 
			continue;
		    }
		    fixEvictPosition(element);
		    
		    synchronized(element)
		    {
			switch(element.status)
			{
			    case EvictorElement.clean:
			    {
				element.status = EvictorElement.modified;
				element.rec.servant = servant;
				addToModifiedQueue(element);
				break;
			    }
			    case EvictorElement.created:
			    case EvictorElement.modified:
			    {
				element.rec.servant = servant;
				break;
			    }
			    case EvictorElement.destroyed:
			    {
				element.status = EvictorElement.modified;
				element.rec.servant = servant;
				
				//
				// No need to push it on the modified queue, as a destroyed object
				// is either already on the queue or about to be saved. When saved,
				// it becomes dead.
				//
				break;
			    }
			    case EvictorElement.dead:
			    {
				element.status = EvictorElement.created;
				ObjectRecord rec = element.rec;
				
				rec.servant = servant;
				rec.stats.creationTime = System.currentTimeMillis();
				rec.stats.lastSaveTime = 0;
				rec.stats.avgSaveTime = 0;
				
				addToModifiedQueue(element);
				break;
			    }
			    default:
			    {
				assert false;
				break;
			    }
			}
		    }
		}
		break; // for(;;)
	    }
	    
	    if(_trace >= 1)
	    {
		String objString = "object \"" + Ice.Util.identityToString(ident) + "\"";
		_communicator.getLogger().trace(
		    "Freeze.Evictor", 
		    "added or updated " + objString + " in the database");
	    }
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }

    
    public Ice.Object
    remove(Ice.Identity ident)
    {
	return removeFacet(ident, "");
    }

    public Ice.Object
    removeFacet(Ice.Identity ident, String facet)
    {
	checkIdentity(ident);
	if(facet == null)
        {
            facet = "";
        }

	//
	// Need to clone in case the given ident changes.
	//
	try
	{
	    ident = (Ice.Identity) ident.clone();
	}
	catch(CloneNotSupportedException ex)
	{
	    assert false;
	}
	
	_deactivateController.lock();

	try
	{    
	    ObjectStore store = findStore(facet);
	    Ice.Object servant = null;
	    
	    if(store != null)
	    {
		for(;;)
		{
		    //
		    // Retrieve object
		    //
		    
		    EvictorElement element = (EvictorElement) store.cache().pin(ident);
		    if(element != null)
		    {
			synchronized(this)
			{
			    if(element.stale)
			    {
				//
				// Try again
				// 
				continue;
			    }
			    
			    fixEvictPosition(element);
			    synchronized(element)
			    {
				switch(element.status)
				{
				    case EvictorElement.clean:
				    {
					servant = element.rec.servant;
					element.status = EvictorElement.destroyed;
					element.rec.servant = null;
					addToModifiedQueue(element);
					break;
				    }
				    case EvictorElement.created:
				    {
					servant = element.rec.servant;
					element.status = EvictorElement.dead;
					element.rec.servant = null;
					break;
				    }
				    case EvictorElement.modified:
				    {
					servant = element.rec.servant;
					element.status = EvictorElement.destroyed;
					element.rec.servant = null;
					//
					// Not necessary to push it on the modified queue, as a modified
					// element is either on the queue already or about to be saved
					// (at which point it becomes clean)
					//
					break;
				    }  
				    case EvictorElement.destroyed:
				    case EvictorElement.dead:
				    {
					break;
				    }
				    default:
				    {
					assert false;
					break;
				    }
				}
			    }
			    
			    if(element. keepCount > 0)
			    {
				assert servant != null;
				
				element.keepCount = 0;
				//
				// Add to front of evictor queue
				//
				// Note that save evicts dead objects
				//
				_evictorList.addFirst(element);
				element.evictPosition = _evictorList.iterator();
				//
				// Position the iterator "on" the element.
				//
				element.evictPosition.next();
				_currentEvictorSize++;
			    }
			}
		    }
		    break; // for(;;)  
		}
	    }
	    
	    if(servant == null)
	    {
		Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
		ex.kindOfObject = "servant";
		ex.id = Ice.Util.identityToString(ident);
		if(facet.length() > 0)
		{
		    ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
		}
		throw ex;
	    }
	    
	    if(_trace >= 1)
	    {
		String objString = "object \"" + Ice.Util.identityToString(ident) + "\"";
		if(!facet.equals(""))
		{
		    objString += " with facet \"" + facet + "\"";
		}
		
		_communicator.getLogger().trace(
		    "Freeze.Evictor", 
		    "removed " + objString);
	    }
	    return servant;
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }

    //
    // Deprecated
    //
    public void
    destroyObject(Ice.Identity ident)
    {
	checkIdentity(ident);

	try
	{
	    remove(ident);
	}
	catch(Ice.NotRegisteredException e)
	{
	    //
	    // Ignored
	    //
	}
    }

    
    public void
    keep(Ice.Identity ident)
    {
	keepFacet(ident, "");
    }

    public void
    keepFacet(Ice.Identity ident, String facet)
    {
	checkIdentity(ident);
	if(facet == null)
        {
            facet = "";
        }

	_deactivateController.lock();

	try
	{
	    boolean notThere = false;
	    
	    ObjectStore store = findStore(facet);
	    if(store == null)
	    {
		notThere = true;
	    }
	    else
	    {
		for(;;)
		{
		    EvictorElement element = (EvictorElement) store.cache().pin(ident);
		    if(element == null)
		    {
			notThere = true;
			break;
		    }
		    
		    synchronized(this)
		    {
			if(element.stale)
			{
			    //
			    // try again
			    //
			    continue;
			}
			
			synchronized(element)
			{
			    if(element.status == EvictorElement.destroyed || element.status == EvictorElement.dead)
			    {
				notThere = true;
				break;
			    }
			}
			
			//
			// Found!
			//
			
			if(element.keepCount == 0)
			{
			    if(element.usageCount < 0)
			    {
				//
				// New object
				//
				element.usageCount = 0;
			    }
			    else
			    {
				assert element.evictPosition != null;
				element.evictPosition.remove();
				element.evictPosition = null;
				_currentEvictorSize--;
			    }
			    element.keepCount = 1;
			}
			else
			{
			    element.keepCount++;
			}
			break;
		    }
		}
	    }
	    
	    if(notThere)
	    {
		Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
		ex.kindOfObject = "servant";
		ex.id = Ice.Util.identityToString(ident);
		if(facet.length() > 0)
		{
		    ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
		}
		throw ex;
	    }
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }

    public void
    release(Ice.Identity ident)
    {
	releaseFacet(ident, "");
    }

    public void
    releaseFacet(Ice.Identity ident, String facet)
    {
	checkIdentity(ident);
	if(facet == null)
        {
            facet = "";
        }

	_deactivateController.lock();

	try
	{
	    synchronized(this)
	    {
		ObjectStore store = (ObjectStore) _storeMap.get(facet);
		
		if(store != null)
		{    
		    EvictorElement element = (EvictorElement) store.cache().getIfPinned(ident);
		    if(element != null)
		    {
			assert !element.stale;
			if(element.keepCount > 0) 
			{
			    if(--element.keepCount == 0)
			    {
				//
				// Add to front of evictor queue
				//
				// Note that the element cannot be destroyed or dead since
				// its keepCount was > 0.
				//
				assert element.evictPosition == null;
				_evictorList.addFirst(element);
				element.evictPosition = _evictorList.iterator();
				//
				// Position the iterator "on" the element.
				//
				element.evictPosition.next();
				_currentEvictorSize++;
			    }
			    //
			    // Success
			    //
			    return;
			}
		    }
		}
	    }
	    
	    Ice.NotRegisteredException ex = new Ice.NotRegisteredException();
	    ex.kindOfObject = "servant";
	    ex.id = Ice.Util.identityToString(ident);
	    if(facet.length() > 0)
	    {
		ex.id += " -f " + IceUtil.StringUtil.escapeString(facet, "");
	    }
	    
	    throw ex;
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }
    

    public EvictorIterator
    getIterator(String facet, int batchSize)
    {
	if(facet == null)
        {
            facet = "";
        }

	_deactivateController.lock();

	try
	{
	    ObjectStore store = null;
	    synchronized(this)
	    {
		store = (ObjectStore) _storeMap.get(facet);
		if(store != null)
		{
		    saveNowNoSync();
		}
	    }
	    return new EvictorIteratorI(store, batchSize);
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }

    public boolean
    hasObject(Ice.Identity ident)
    {
	return hasFacet(ident, "");
    }

    public boolean
    hasFacet(Ice.Identity ident, String facet)
    {
	_deactivateController.lock();
	try
	{
	    return hasFacetImpl(ident, facet);
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }
    
    private boolean
    hasFacetImpl(Ice.Identity ident, String facet)
    {
	//
	// Must be called with _deactivateController locked.
	//

	checkIdentity(ident);
	if(facet == null)
	{
	    facet = "";
	}

	ObjectStore store = null;
	    
	synchronized(this)
	{	
	    store = (ObjectStore) _storeMap.get(facet);
	    if(store == null)
	    {
		return false;
	    }
	    
	    EvictorElement element = (EvictorElement) store.cache().getIfPinned(ident);
	    if(element != null)
	    {
		assert !element.stale;    
		
		synchronized(element)
		{
		    return element.status != EvictorElement.dead && 
			element.status != EvictorElement.destroyed;
		}
	    }
	}
	return store.dbHasObject(ident);
    }

    private boolean
    hasAnotherFacet(Ice.Identity ident, String facet)
    {
	//
	// Must be called with _deactivateController locked.
	//

	//
	// If the object exists in another store, throw FacetNotExistException 
	// instead of returning null (== ObjectNotExistException)
	// 
	java.util.Map storeMapCopy;
	synchronized(this)
	{
	    storeMapCopy = new java.util.HashMap(_storeMap);
	}	    
	
	java.util.Iterator p = storeMapCopy.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry) p.next();
	    
	    //
	    // Do not check facet
	    //
	    if(!facet.equals(entry.getKey()))
	    {
		ObjectStore store = (ObjectStore) entry.getValue();
		boolean inCache = false;
		
		synchronized(this)
		{
		    EvictorElement element = (EvictorElement) store.cache().getIfPinned(ident);
		    if(element != null)
		    {
			inCache = true;
			assert !element.stale;    
			
			synchronized(element)
			{
			    if(element.status != EvictorElement.dead && 
			       element.status != EvictorElement.destroyed)
			    {
				return true;
			    }
			}
		    }
		}
		if(!inCache)
		{
		    if(store.dbHasObject(ident))
		    {
			return true;
		    }
		}
	    }   
	}
	return false;
    }

    public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
	_deactivateController.lock();
	try
	{
	    
	    //
	    // Special ice_ping() handling
	    //
	    if(current.operation != null && current.operation.equals("ice_ping"))
	    {
		assert current.mode == Ice.OperationMode.Nonmutating;

		if(hasFacetImpl(current.id, current.facet))
		{
		    if(_trace >= 3)
		    {
			_communicator.getLogger().trace(
			    "Freeze.Evictor",
			    "ice_ping found \"" + Ice.Util.identityToString(current.id)  
			    + "\" with facet \"" + current.facet + "\"");
		    }
		    cookie.value = null;
		    return _pingObject;
		}
		else if(hasAnotherFacet(current.id, current.facet))
		{
		    if(_trace >= 3)
		    {
			_communicator.getLogger().trace(
			    "Freeze.Evictor",
			    "ice_ping raises FacetNotExistException for \"" + Ice.Util.identityToString(current.id)  
			    + "\" with facet \"" + current.facet + "\"");
		    }
		    
		    throw new Ice.FacetNotExistException();
		}
		else
		{
		    if(_trace >= 3)
		    {
			_communicator.getLogger().trace(
			    "Freeze.Evictor",
			    "ice_ping will raise ObjectNotExistException for \"" + Ice.Util.identityToString(current.id)  
			    + "\" with facet \"" + current.facet + "\"");
		    }
		    
		    return null;
		}
	    }
	    

	    Ice.Object result = locateImpl(current, cookie);
	    
	    if(result == null)
	    {
		if(hasAnotherFacet(current.id, current.facet))
		{
		    throw new Ice.FacetNotExistException();
		}
	    }
		
	    return result;
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }
	
    Ice.Object
    locateImpl(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
	 cookie.value = null;
	 
	 //
	 // Need to clone as current.id gets reused
	 //
	 Ice.Identity ident = null;
	 try
	 {
	      ident = (Ice.Identity) current.id.clone();
	 }
	 catch(CloneNotSupportedException ex)
	 {
	     assert false;
	 }

	 ObjectStore store = findStore(current.facet);
	 if(store == null)
	 {
	     if(_trace >= 2)
	     {
		 _communicator.getLogger().trace(
		     "Freeze.Evictor",
		     "locate could not find a database for facet \"" + current.facet + "\"");
	     }
	     return null;
	 }
	 
	 for(;;)
	 {
	     EvictorElement element = (EvictorElement) store.cache().pin(ident);
	     if(element == null)
	     {
		 if(_trace >= 2)
		 {
		     _communicator.getLogger().trace(
			 "Freeze.Evictor",
			 "locate could not find \"" + Ice.Util.identityToString(ident) 
			 + "\" in database \"" + current.facet + "\"");
		 }
		 return null;
	     }
	     
	     synchronized(this)
	     {	     
		 if(element.stale)
		 {
		     //
		     // try again
		     //
		     continue;
		 }
	     
		 synchronized(element)
		 {
		     if(element.status == EvictorElement.destroyed || 
			element.status == EvictorElement.dead)
		     {
			 if(_trace >= 2)
			 {
			     _communicator.getLogger().trace(
				 "Freeze.Evictor",
				 "locate found \"" + Ice.Util.identityToString(ident) 
				 + "\" in the cache for database \"" + current.facet 
				 + "\" but it was dead or destroyed");
			 }
			 return null;
		     }
		 
		     //
		     // It's a good one!
		     //
		     
		     
		     if(_trace >= 2)
		     {
			 _communicator.getLogger().trace(
			     "Freeze.Evictor",
			     "locate found \"" + Ice.Util.identityToString(ident) 
			     + "\" in database \"" + current.facet + "\"");
		     }
		     
		     fixEvictPosition(element);
		     element.usageCount++;
		     cookie.value = element;
		     assert element.rec.servant != null;
		     return element.rec.servant;
		 }
	     }
	 } 
    }

    public void
    finished(Ice.Current current, Ice.Object servant, Ice.LocalObject cookie)
    {
	assert servant != null;

	_deactivateController.lock();

	try
	{
	    if(cookie != null)
	    {
		EvictorElement element = (EvictorElement) cookie;
		
		boolean enqueue = false;
		
		if(current.mode != Ice.OperationMode.Nonmutating)
		{
		    synchronized(element)
		    {
			if(element.status == EvictorElement.clean)
			{
			    //
			    // Assume this operation updated the object
			    // 
			    element.status = EvictorElement.modified;
			    enqueue = true;
			}
		    }
		}
		
		synchronized(this)
		{
		    //
		    // Only elements with a usageCount == 0 can become stale and we own 
		    // one count!
		    // 
		    assert !element.stale;
		    assert element.usageCount >= 1;
		    
		    //
		    // Decrease the usage count of the evictor queue element.
		    //
		    element.usageCount--;
		    
		    if(enqueue)
		    {
			addToModifiedQueue(element);
		    }
		    else if(element.usageCount == 0 && element.keepCount == 0)
		    {
			//
			// Evict as many elements as necessary.
			//
			evict();
		    }
		}
	    }
	}
	finally
	{
	    _deactivateController.unlock();
	}
    }

    public void
    deactivate(String category)
    {
	if(_deactivateController.deactivate())
	{
	    try
	    {
		synchronized(this)
		{
		    saveNowNoSync();
		    
		    //
		    // Set the evictor size to zero, meaning that we will evict
		    // everything possible.
		    //
		    _evictorSize = 0;
		    evict();
		    
		    _savingThreadDone = true;
		    notifyAll();
		}
		
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

		if(_watchDogThread != null)
		{
		    _watchDogThread.terminate();
		    
		    for(;;)
		    {
			try
			{
			    _watchDogThread.join();
			    break;
			}
			catch(InterruptedException ex)
			{
			}
		    }	
		}
		
		java.util.Iterator p = _storeMap.values().iterator();
		while(p.hasNext())
		{
		    ObjectStore store = (ObjectStore) p.next();
		    store.close();
		}
		
		if(_dbEnv != null)
		{
		    try
		    {
			_dbEnv.close();
		    }
		    finally
		    {
			_dbEnv = null;
		    }
		}
	    }
	    finally
	    {
		_deactivateController.deactivationComplete();
	    }
	}
    }

    void 
    initialize(Ice.Identity ident, String facet, Ice.Object servant)
    {
	if(_initializer != null)
	{
	    _initializer.initialize(_adapter, ident, facet, servant);
	}
    }

    public void
    run()
    {
	try
	{
	    for(;;)
	    {
		java.util.List allObjects;
		java.util.List deadObjects = new java.util.LinkedList();

		int saveNowThreadsSize = 0;
		
		synchronized(this)
		{
		    while(!_savingThreadDone &&
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
		    
		    if(_savingThreadDone)
		    {
			assert(_modifiedQueue.size() == 0);
			assert(saveNowThreadsSize == 0);
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
		
		long streamStart = System.currentTimeMillis();
		
		//
		// Stream each element
		//
		for(int i = 0; i < size; i++)
		{
		    EvictorElement element = (EvictorElement) allObjects.get(i);
		    
		    boolean tryAgain;
		    
		    do
		    {
			tryAgain = false;
			Ice.Object servant = null;
			
			synchronized(element)
			{
			    byte status = element.status;
			    
			    switch(status)
			    {
				case EvictorElement.created:
				case EvictorElement.modified:
				{
				    servant = element.rec.servant;
				    break;
				}   
				case EvictorElement.destroyed:
				{
				    streamedObjectQueue.add(stream(element, streamStart));

				    element.status = EvictorElement.dead;
				    deadObjects.add(element);
				    break;
				}   
				case EvictorElement.dead:
				{
				    deadObjects.add(element);
				    break;
				}
				default:
				{
				    //
				    // Nothing to do (could be a duplicate)
				    //
				    break;
				}
			    }
			}
			
			if(servant != null)
			{
			    //
			    // Lock servant and then facet so that user can safely lock
			    // servant and call various Evictor operations
			    //
			    if(_watchDogThread != null)
			    {
				_watchDogThread.activate();
			    }
			    synchronized(servant)
			    {
				if(_watchDogThread != null)
				{
				    _watchDogThread.deactivate();
				}

				synchronized(element)
				{
				    byte status = element.status;
				    
				    switch(status)
				    {
					case EvictorElement.created:
					case EvictorElement.modified:
					{
					    if(servant == element.rec.servant)
					    {
						streamedObjectQueue.add(stream(element, streamStart));

						element.status = EvictorElement.clean;
					    }
					    else
					    {
						tryAgain = true;
					    }
					    break;
					}
					case EvictorElement.destroyed:
					{
					    streamedObjectQueue.add(stream(element, streamStart));
					    
					    element.status = EvictorElement.dead;
					    deadObjects.add(element);
					    break;
					}   
					case EvictorElement.dead:
					{
					    deadObjects.add(element);
					    break;
					}
					default:
					{
					    //
					    // Nothing to do (could be a duplicate)
					    //
					    break;
					}
				    }
				}
			    }
			}
		    } while(tryAgain);
		}
		
		if(_trace >= 1)
		{
		    long now = System.currentTimeMillis();
		    _communicator.getLogger().trace(
			"Freeze.Evictor",
			"streamed " + streamedObjectQueue.size() + " objects in " + (now - streamStart) + " ms");
		}
		
		//
		// Now let's save all these streamed objects to disk using a transaction
		//
		
		//
		// Each time we get a deadlock, we reduce the number of objects to save
		// per transaction
		//
		int txSize = streamedObjectQueue.size();
		if(txSize > _maxTxSize)
		{
		    txSize = _maxTxSize;
		}
		
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
			
			long saveStart = System.currentTimeMillis();
			String txnId = null;

			try
			{
			    com.sleepycat.db.DbTxn tx = _dbEnv.getEnv().txnBegin(null, 0);

			    if(_txTrace >= 1)
			    {
				txnId = Long.toHexString((tx.id() & 0x7FFFFFFF) + 0x80000000L); 

				_communicator.getLogger().trace
				    ("Freeze.Evictor", _errorPrefix + "started transaction " +
				     txnId + " in saving thread");
			    }

			    try
			    {   
				for(int i = 0; i < txSize; i++)
				{
				    StreamedObject obj = (StreamedObject) streamedObjectQueue.get(i);
				    obj.store.save(obj.key, obj.value, obj.status, tx);
				}
				
				com.sleepycat.db.DbTxn toCommit = tx;
				tx = null;
				toCommit.commit(0);

				if(_txTrace >= 1)
				{
				    _communicator.getLogger().trace
					("Freeze.Evictor", _errorPrefix + "committed transaction " +
					 txnId);
				}
			    }
			    finally
			    {
				if(tx != null)
				{
				    tx.abort();
				    if(_txTrace >= 1)
				    {
					_communicator.getLogger().trace
					    ("Freeze.Evictor", _errorPrefix + "rolled back transaction " +
					     txnId);
				    }
				}
			    }
			    
			    for(int i = 0; i < txSize; i++)
			    {
				streamedObjectQueue.remove(0);
			    }
			    
			    if(_trace >= 1)
			    {
				long now = System.currentTimeMillis();
				_communicator.getLogger().trace(
				    "Freeze.Evictor",
				    "saved " + txSize + " objects in " + (now - saveStart) + " ms");
			    }
			}
			catch(com.sleepycat.db.DbDeadlockException deadlock)
			{
			    if(_deadlockWarning)
			    {
				_communicator.getLogger().warning
				    ("Deadlock in Freeze.EvictorI.run while writing into Db \"" + _filename
				     + "\"; retrying ...");
			    }
			    
			    tryAgain = true;
			    txSize = (txSize + 1)/2;
			}
			catch(com.sleepycat.db.DbException dx)
			{
			    DatabaseException ex = new DatabaseException();
			    ex.initCause(dx);
			    ex.message = _errorPrefix + "saving: " + dx.getMessage();
			    throw ex;
			}
		    } 
		} while(tryAgain);
		
		synchronized(this)
		{  
		    //
		    // Release usage count
		    //
		    for(int i = 0; i < allObjects.size(); i++)
		    {    
			EvictorElement element = (EvictorElement) allObjects.get(i);
			assert element.usageCount > 0;
			element.usageCount--;
		    }
		    allObjects.clear();

		    java.util.Iterator p = deadObjects.iterator();
		    while(p.hasNext())
		    {
			EvictorElement element = (EvictorElement) p.next();

			//
			// Can be stale when there are duplicates on the deadObjects list
			//
			if(!element.stale && element.usageCount == 0 && element.keepCount == 0)
			{
			    //
			    // Get rid of unused dead elements
			    //
			    synchronized(element)
			    {
				if(element.status == EvictorElement.dead)
				{
				    evict(element);
				}
			    }
			}
		    }

		    deadObjects.clear();
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
	    }
	}
	catch(RuntimeException ex)
	{
	    java.io.StringWriter sw = new java.io.StringWriter();
	    java.io.PrintWriter pw = new java.io.PrintWriter(sw);
	    ex.printStackTrace(pw);
	    pw.flush();
	    _communicator.getLogger().
		error(_errorPrefix + "Fatal error in saving thread:\n" + sw.toString());

	    Util.handleFatalError(this, _communicator, ex);
	}
    }


    final DeactivateController
    deactivateController()
    {
	return _deactivateController;
    }

    final Ice.Communicator
    communicator()
    {
	return _communicator;
    }

    final SharedDbEnv
    dbEnv()
    {
	return _dbEnv;
    }

    final String
    filename()
    {
	return _filename;
    }

    final String
    errorPrefix()
    {
	return _errorPrefix;
    }

    final int
    txTrace()
    {
	return _txTrace;
    }
    
    final boolean
    deadlockWarning()
    {
	return _deadlockWarning;
    }    

    synchronized void
    saveNow()
    {	
	saveNowNoSync();
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
    
    private void
    evict()
    {
	assert Thread.holdsLock(this);

	java.util.Iterator p = _evictorList.riterator();
	while(p.hasNext() && _currentEvictorSize > _evictorSize)
	{
	    //
	    // Get the last unused element from the evictor queue.
	    //
	    EvictorElement element = (EvictorElement)p.next();
	    if(element.usageCount == 0)
	    {
		//
		// Fine, servant is not in use (and not in the modifiedQueue)
		//
		
		assert !element.stale;
		assert element.keepCount == 0;
		assert element.evictPosition != null;

		if(_trace >= 2 || (_trace >= 1 && _evictorList.size() % 50 == 0))
		{
		    String objString = "object \"" + Ice.Util.identityToString(element.identity) + "\"";
		    String facet = element.store.facet();
		    if(facet.length() > 0)
		    {
			objString += " with facet \"" + facet + "\"";
		    }

		    _communicator.getLogger().trace(
			"Freeze.Evictor", 
			"evicting " + objString + " from the queue; " + "number of elements in the queue: " +
			_currentEvictorSize);
		}	
		
		//
		// Remove last unused element from the evictor queue.
		//
		element.stale = true;
		element.store.cache().unpin(element.identity);
		p.remove();
		element.evictPosition = null;
		_currentEvictorSize--;	
	    }
	}
    }

    
    private void 
    fixEvictPosition(EvictorElement element)
    {
	assert Thread.holdsLock(this);

	assert !element.stale;

	if(element.keepCount == 0)
	{
	    if(element.usageCount < 0)
	    {
		assert element.evictPosition == null;
		
		//
		// New object
		//
		element.usageCount = 0;
		_currentEvictorSize++;
	    }
	    else
	    {
		assert element.evictPosition != null;
		element.evictPosition.remove();
	    }
	    _evictorList.addFirst(element);
	    element.evictPosition = _evictorList.iterator();
	    //
	    // Position the iterator "on" the element.
	    //
	    element.evictPosition.next();
	}
    }

    private void
    evict(EvictorElement element)
    {
	assert Thread.holdsLock(this);

	assert !element.stale;
	assert element.keepCount == 0;
	
	element.evictPosition.remove();
	_currentEvictorSize--;
	element.stale = true;
	element.store.cache().unpin(element.identity);
    }


    private void
    addToModifiedQueue(EvictorElement element)
    {
	assert Thread.holdsLock(this);

	element.usageCount++;
	_modifiedQueue.add(element);
	
	if(_saveSizeTrigger >= 0 && _modifiedQueue.size() >= _saveSizeTrigger)
	{
	    notifyAll();
	}
    }

    private StreamedObject
    stream(EvictorElement element, long streamStart)
    {
	assert Thread.holdsLock(element);

	assert element.status != EvictorElement.dead;
    
	StreamedObject obj = new StreamedObject();

	obj.status = element.status;
	obj.store = element.store;
	obj.key = ObjectStore.marshalKey(element.identity, _communicator);
	
	if(element.status != EvictorElement.destroyed)
	{
	    //
	    // Update stats first
	    //
	    Statistics stats = element.rec.stats;
	    long diff = streamStart - (stats.creationTime + stats.lastSaveTime);
	    if(stats.lastSaveTime == 0)
	    {
		stats.lastSaveTime = diff;
		stats.avgSaveTime = diff;
	    }
	    else
	    {
		stats.lastSaveTime = streamStart - stats.creationTime;
		stats.avgSaveTime = (long)(stats.avgSaveTime * 0.95 + diff * 0.05);
	    }
	    obj.value = ObjectStore.marshalValue(element.rec, _communicator);
	}
	return obj;
    }
        
    private synchronized ObjectStore
    findStore(String facet)
    {
	return (ObjectStore) _storeMap.get(facet);
    }


    private java.util.List
    allDbs()
    {
	java.util.List result = new java.util.LinkedList();
	
	com.sleepycat.db.Db db = null;
	com.sleepycat.db.Dbc dbc = null;

	try
	{
	    db = new com.sleepycat.db.Db(_dbEnv.getEnv(), 0);
	    db.open(null, _filename, null, com.sleepycat.db.Db.DB_UNKNOWN, com.sleepycat.db.Db.DB_RDONLY, 0);
	 
	    dbc = db.cursor(null, 0);
	    
	    com.sleepycat.db.Dbt key = new com.sleepycat.db.Dbt();
	    key.setFlags(com.sleepycat.db.Db.DB_DBT_MALLOC);
	    
	    com.sleepycat.db.Dbt value = new com.sleepycat.db.Dbt();
	    value.setFlags(com.sleepycat.db.Db.DB_DBT_MALLOC);
	    
	    boolean more = true;
	    while(more)
	    {
		more = (dbc.get(key, value, com.sleepycat.db.Db.DB_NEXT) == 0);
		if(more)
		{
		    //
		    // Assumes Berkeley-DB encodes the db names in UTF-8!
		    //
		    String dbName = new String(key.getData(), 0, key.getSize(), "UTF8");
		
		    if(!dbName.startsWith(indexPrefix))
		    {
			result.add(dbName);
		    }
		}
	    }

	    dbc.close();
	    dbc = null;
	    db.close(0);
	    db = null;
	}
	catch(java.io.UnsupportedEncodingException ix)
	{
	    DatabaseException ex = new DatabaseException();
	    ex.initCause(ix);
	    ex.message = _errorPrefix + "cannot decode database names";
	    throw ex;
	}
	catch(java.io.FileNotFoundException ix)
	{
	    //
	    // New file
	    //
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    DatabaseException ex = new DatabaseException();
	    ex.initCause(dx);
	    ex.message = _errorPrefix + "Db.open: " + dx.getMessage();
	    throw ex;
	}
	finally
	{
	    if(dbc != null)
	    {
		try
		{
		    dbc.close();
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    // Ignored
		}
	    }

	    if(db != null)
	    {
		try
		{
		    db.close(0);
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    // Ignored
		}
	    }
	}

	return result;
    }
    

    private static void
    checkIdentity(Ice.Identity ident)
    {
        if(ident.name == null || ident.name.length() == 0)
        {
            Ice.IllegalIdentityException e = 
		new Ice.IllegalIdentityException();
            try
            {
                e.id = (Ice.Identity)ident.clone();
            }
            catch(CloneNotSupportedException ex)
            {
                assert(false);
            }
            throw e;
        }
    }

    static class StreamedObject
    {
	byte[] key = null;
	byte[] value = null;
	byte status = EvictorElement.dead;
	ObjectStore store = null;
    }

    //
    // Map of string (facet) to ObjectStore
    //
    private final java.util.Map _storeMap = new java.util.HashMap();
    
    //
    // List of EvictorElement with stable iterators
    //
    private final Freeze.LinkedList _evictorList = new Freeze.LinkedList();
    private int _evictorSize = 10;
    private int _currentEvictorSize = 0;

    //
    // The _modifiedQueue contains a queue of all modified facets
    // Each element in the queue "owns" a usage count, to ensure the
    // elements containing them remain in the map.
    //
    private java.util.List _modifiedQueue = new java.util.ArrayList();
    
    private boolean              _savingThreadDone = false;
    private WatchDogThread       _watchDogThread = null;

    private DeactivateController _deactivateController = new DeactivateController();
   
    private final Ice.ObjectAdapter _adapter;
    private final Ice.Communicator _communicator;

    private final ServantInitializer _initializer;

    private SharedDbEnv  _dbEnv;

    private final String _filename;
    private final boolean _createDb;

    private int _trace = 0;
    private int _txTrace = 0;

    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    private final java.util.List _saveNowThreads = new java.util.ArrayList();

    private int _saveSizeTrigger;
    private int _maxTxSize;
    private long _savePeriod;

    private Thread _thread;

    private String _errorPrefix;
    
    private boolean _deadlockWarning;    

    private Ice.Object _pingObject = new Ice.ObjectImpl();

}
