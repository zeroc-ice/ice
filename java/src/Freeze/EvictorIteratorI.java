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
    
    EvictorIteratorI(EvictorI evictor, int batchSize, boolean loadServants)
    {
	_evictor = evictor;
	_batchSize = batchSize;
	_loadServants = loadServants;
	
	assert(batchSize > 0);

	_key.set_flags(com.sleepycat.db.Db.DB_DBT_REALLOC);

	if(_loadServants)
	{
	    _value.set_flags(com.sleepycat.db.Db.DB_DBT_REALLOC);
	}
	else
	{
	    //
	    // dlen is 0, so we should not retrieve any value 
	    // 
	    _value.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	}
    }

    private java.util.Iterator
    nextBatch()
    {
	if(!_more)
	{
	    return null;
	}

	java.util.List evictorElements = null;

	Ice.Communicator communicator = _evictor.communicator();

	int loadedGeneration = 0;

	try
	{
	    for(;;)
	    {
		com.sleepycat.db.Dbc dbc = null;
		
		_batch = new java.util.ArrayList(); 
		if(_loadServants)
		{
		    evictorElements = new java.util.ArrayList();
		}
		
		int count = _batchSize;
		
		try
		{
		    //
		    // Move to the first record
		    // 
		    int flags = com.sleepycat.db.Db.DB_NEXT;
		    if(_key.get_size() > 0)
		    {
			//
			// _key represents the next element not yet returned
			// if it has been deleted, we want the one after
			//
			flags = com.sleepycat.db.Db.DB_SET_RANGE;
		    }

		    if(_loadServants)
		    {
			loadedGeneration = _evictor.currentGeneration();
		    }
		    dbc = _evictor.db().cursor(null, 0);
		    _more = (dbc.get(_key, _value, flags) == 0);
		    
		    while(count > 0 && _more)
		    {
			EvictorStorageKey esk = EvictorI.unmarshalKey(_key.get_data(), communicator);
			
			//
			// Because of the Ice encoding and default binary comparison, records with
			// facet length = 0 are before records with facet length > 0 (for a given
			// identity).
			//
			assert(esk.facet.length == 0);
			
			Ice.Identity ident = esk.identity;
			_batch.add(ident);
			count--;
			
			//
			// Even when count is 0, we read one more record (unless we reach the end)
			//
			if(_loadServants)
			{
			    _more = _evictor.load(ident, dbc, _key, _value, evictorElements);
			}
			else
			{
			    _more = _evictor.skipFacets(ident, dbc, _key, _value);
			}
		    }
		    break; // for (;;)
		}
		catch(com.sleepycat.db.DbDeadlockException dx)
		{
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
	    ex.message = _evictor.errorPrefix() + "Db.cursor: " + dx.getMessage();
	    throw ex;
	}

	if(_batch.size() == 0)
	{
	    return null;
	}
	else
	{
	    if(_loadServants)
	    {
		_evictor.insert(_batch, evictorElements, loadedGeneration);
	    }
	    return _batch.listIterator();
	}
    }

    private EvictorI _evictor;
    private int _batchSize;
    private boolean _loadServants;
    private java.util.Iterator _batchIterator;

    private com.sleepycat.db.Dbt _key = new com.sleepycat.db.Dbt();
    private com.sleepycat.db.Dbt _value = new com.sleepycat.db.Dbt();
    private java.util.List _batch = null;
    private boolean _more = true;
}
