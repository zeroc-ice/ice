// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class EvictorIteratorI extends Ice.LocalObjectImpl implements EvictorIterator
{
    public boolean
    hasNext()
    {
	if(_batchIterator != null && _batchIterator.hasNext()) 
	{
	    return true;
	}
	else
	{
	    _batchIterator = nextBatch();
	    return (_batchIterator != null);
	}
    }

    public Ice.Identity
    next()
    {
	if(hasNext())
	{
	    return (Ice.Identity) _batchIterator.next();
	}
	else
	{
	    throw new NoSuchElementException();
	}
    }
    
    EvictorIteratorI(ObjectStore store, int batchSize)
    {
	_store = store;
	_more = (store != null);
	_batchSize = batchSize;
	
	assert batchSize > 0;

	//
	// We should use DB_DBT_REALLOC, but it's buggy in 4.1.25 
	// (causes weird problems, e.g. can't find some values)
	//
	_key.setFlags(com.sleepycat.db.Db.DB_DBT_MALLOC);

	//
	// dlen is 0, so we should not retrieve any value 
	// 
	_value.setFlags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
    }

    private java.util.Iterator
    nextBatch()
    {
	EvictorI.DeactivateController deactivateController = _store.evictor().deactivateController();
	deactivateController.lock();
	
	try
	{
	    if(!_more)
	    {
		return null;
	    }
	    
	    java.util.List evictorElements = null;
	    
	    Ice.Communicator communicator = _store.communicator();
	    
	    byte[] firstKey = null;
	    if(_key.getSize() > 0)
	    {
		firstKey = new byte[_key.getSize()];
		System.arraycopy(_key.getData(), 0, firstKey, 0, firstKey.length);
	    }
	    
	    try
	    {
		for(;;)
		{
		    com.sleepycat.db.Dbc dbc = null;
		    
		    _batch = new java.util.ArrayList(); 
		    
		    try
		    {
			//
			// Move to the first record
			// 
			int flags = com.sleepycat.db.Db.DB_NEXT;
			if(firstKey != null)
			{
			    //
			    // _key represents the next element not yet returned
			    // if it has been deleted, we want the one after
			    //
			    flags = com.sleepycat.db.Db.DB_SET_RANGE;
			}
			
			dbc = _store.db().cursor(null, 0);
			
			boolean done = false;
			do
			{
			    _more = (dbc.get(_key, _value, flags) == 0);
			    
			    if(_more)
			    {
				flags = com.sleepycat.db.Db.DB_NEXT;
				
				if(_batch.size() < _batchSize)
				{
				    Ice.Identity ident = ObjectStore.unmarshalKey(_key.getData(), communicator);
				    _batch.add(ident);
				}
				else
				{
				    //
				    // Keep the last element in _key
				    //
				    done = true;
				}
			    }
			}
			while(!done && _more);
			
			break; // for (;;)
		    }
		    catch(com.sleepycat.db.DbDeadlockException dx)
		    {
			if(firstKey != null)
			{
			    assert(_key.getData().length >= firstKey.length);
			    System.arraycopy(firstKey, 0, _key.getData(), 0, firstKey.length);
			    _key.setSize(firstKey.length);
			}
			else
			{
			    _key.setSize(0);
			}
			
			if(_store.evictor().deadlockWarning())
			{
			    communicator.getLogger().warning
				("Deadlock in Freeze.EvictorIteratorI.load while iterating over Db \"" 
				 + _store.evictor().filename() + "/" + _store.dbName()
				 + "\"; retrying ...");
			}
			
			//
			// Retry
			//
		    }
		    finally
		    {
			if(dbc != null)
			{
			    try
			    {
				dbc.close();
			    }
			    catch(com.sleepycat.db.DbDeadlockException dx)
			    {
				//
				// Ignored
				//
			    }
			}
		    }
		}
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _store.evictor().errorPrefix() + "Db.cursor: " + dx.getMessage();
		throw ex;
	    }
	    
	    if(_batch.size() == 0)
	    {
		return null;
	    }
	    else
	    {
		return _batch.listIterator();
	    }
	}
	finally
	{
	    deactivateController.unlock();
	}
    }

    private final ObjectStore _store;
    private final int _batchSize;
    private java.util.Iterator _batchIterator;

    private final com.sleepycat.db.Dbt _key = new com.sleepycat.db.Dbt();
    private final com.sleepycat.db.Dbt _value = new com.sleepycat.db.Dbt();
    private java.util.List _batch = null;
    private boolean _more = true;
}
