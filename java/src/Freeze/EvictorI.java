// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

package Freeze;

class EvictorI extends Ice.LocalObjectImpl implements Evictor
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
	// Save the new Ice Object to the database.
	//
	_dict.fastPut(identCopy, servant);
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
	// Delete the Ice Object from the database.
	//
	_dict.fastRemove(ident);
	remove(ident);
	
	if(_trace >= 1)
	{
	    _db.getCommunicator().getLogger().trace("Evictor", "destroyed \"" +
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
	// TODO: HACK: It's possible that locate is called on a
	// deactivated servant locator. There's currently no nice way to
	// handle this case so we just through an UnknownLocatoException.
	//
	if(_deactivated)
	{
	    throw new Ice.UnknownLocalException();
	}

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
	    // Ice Object found in evictor map. Push it to the front of
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
	    // Load the Ice Object from database and create and add a
	    // Servant for it.
	    //
	    Ice.Object servant = (Ice.Object)_dict.get(ident);
	    if(servant == null)
	    {
		//
		// The Ice Object with the given identity does not exist,
		// client will get an ObjectNotExistException.
		//
		return null;
	    }
	    
	    //
	    // Add the new Servant to the evictor
	    // queue. current.id is copied
	    //
	    element = add(ident, servant);
	    
	    //
	    // If an initializer is installed, call it now.
	    //
	    if(_initializer != null)
	    {
		_initializer.initialize(current.adapter, ident, servant);
	    }
	}
	
	//
	// Increase the usage count of the evictor queue element.
	//
	++element.usageCount;
	
	//
	// Evict as many elements as necessary.
	//
	evict();
	
	//
	// Set the cookie and return the servant for the Ice Object.
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
	
	//
	// If we are in SaveAfterMutatingOperation mode, we must save the
	// Ice Object if this was a mutating call.
	//
	if(_persistenceMode == EvictorPersistenceMode.SaveAfterMutatingOperation)
	{
	    if(current.mode != Ice.OperationMode.Nonmutating)
	    {
		_dict.fastPut(current.id, servant);
	    }
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
							"deactivating, saving unsaved Ice Objects to the database");
	    }
	    
	    //
	    // Set the evictor size to zero, meaning that we will evict
	    // everything possible.
	    //
	    _evictorSize = 0;
	    evict();
	}
    }

    EvictorI(DB db, EvictorPersistenceMode persistenceMode)
    {
	_db = db;
	_dict = new IdentityObjectDict(db);
	_persistenceMode = persistenceMode;
	_trace = _db.getCommunicator().getProperties().getPropertyAsInt("Freeze.Trace.Evictor");
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
		// Fine, Servant is not in use.
		//
		assert(ident != null && element != null);

		//
		// If we are in SaveUponEviction mode, we must
		// save the Ice Object that is about to be
		// evicted to persistent store.
		//
		if(_persistenceMode == EvictorPersistenceMode.SaveUponEviction)
		{
		    _dict.fastPut(ident, element.servant);
		}
	    
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
	// If we're deactivated, and there are no more elements to
	// evict it's not necessary in Java to set _db to zero to
	// break cyclic dependences.
	//
    }

    private EvictorElement
    add(Ice.Identity ident, Ice.Object servant)
    {
	//
	// Ignore the request if the Ice Object is already in the queue.
	//
	EvictorElement element = (EvictorElement)_evictorMap.get(ident);
	if(element != null)
	{
	    return element;
	}    
	
	//
	// Add an Ice Object with its Servant to the evictor queue.
	//
	_evictorList.addFirst(ident);

	element = new EvictorElement();
	element.servant = servant;
	element.usageCount = 0;    
	element.position = _evictorList.iterator();

	//
	// Position the iterator "on" the element.
	//
	element.position.next();

	_evictorMap.put(ident, element);
	return element;
    }
    
    private void
    remove(Ice.Identity ident)
    {
	//
	// If the Ice Object is currently in the evictor, remove it.
	//
	EvictorElement element = (EvictorElement)_evictorMap.remove(ident);
	if(element != null)
	{
	    element.position.remove();
	}    
    }

    class EvictorElement extends Ice.LocalObjectImpl
    {
	public Ice.Object servant;
	java.util.Iterator position;
	public int usageCount;
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
    private EvictorPersistenceMode _persistenceMode;
    private ServantInitializer _initializer;
    private int _trace = 0;
}
