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
	if(_dbc == null)
	{
	    throw new IteratorDestroyedException();
	}
	
	if(_current != null)
	{
	    return true;
	}
	else
	{
	    com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt();
	
	    //
	    // Keep 0 length since we're not interested in the data
	    // Unfortunately Berkeley DB does not like null data buffers,
	    // so we use a dummy buffer
	    // 
	    com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
	    dbValue.set_flags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	  
	    for(;;)
	    {
		try
		{
		    if(_dbc.get(dbKey, dbValue, com.sleepycat.db.Db.DB_NEXT) == 0)
		    {
			_current = EvictorI.unmarshalKey(dbKey.get_data(), _communicator);
			if(_current.facet.length == 0)
			{
			    return true;
			}
			//
			// Otherwise loop
			//
		    }
		    else
		    {
			return false;
		    }
		}
		catch(com.sleepycat.db.DbDeadlockException dx)
		{
		    DBDeadlockException ex = new DBDeadlockException();
		    ex.initCause(dx);
		    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
		    throw ex;
		}
		catch(com.sleepycat.db.DbException dx)
		{
		    DBException ex = new DBException();
		    ex.initCause(dx);
		    ex.message = _errorPrefix + "Dbc.get: " + dx.getMessage();
		    throw ex;
		}
	    }
	}
    }

    public Ice.Identity
    next()
    {
	if(hasNext())
	{
	    Ice.Identity result = _current.identity;
	    _current = null;
	    return result;
	}
	else
	{
	    throw new NoSuchElementException();
	}
    }

    public void
    destroy()
    {
	if(_dbc == null)
	{
	    throw new IteratorDestroyedException();
	}
	else
	{
	    try
	    {
		_dbc.close();
	    }
	    catch(com.sleepycat.db.DbDeadlockException deadlock)
	    {
		//
		// Ignored
		//
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DBException ex = new DBException();
		ex.initCause(dx);
		ex.message = _errorPrefix + "Db.cursor: " + dx.getMessage();
		throw ex;
	    }
	   
	    _dbc = null;
	    _current = null;
	    _communicator = null;
	}
    }

    EvictorIteratorI(com.sleepycat.db.Db db, Ice.Communicator communicator, String errorPrefix)
    {
	_communicator = communicator;
	_errorPrefix = errorPrefix;

	try
	{
	    _dbc = db.cursor(null, 0);
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    DBException ex = new DBException();
	    ex.initCause(dx);
	    ex.message = _errorPrefix + "Db.cursor: " + dx.getMessage();
	    throw ex;
	}
    }

    protected void
    finalize()
    {
	if(_dbc != null)
	{
	    destroy();
	}
    }


    private com.sleepycat.db.Dbc _dbc;
    private EvictorStorageKey _current = null;
    private Ice.Communicator _communicator;
    private String _errorPrefix;
}
