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

#include <Freeze/EvictorIteratorI.h>
#include <Freeze/EvictorI.h>
#include <Freeze/Util.h>

using namespace std;
using namespace Freeze;
using namespace Ice;


Freeze::EvictorIteratorI::EvictorIteratorI(EvictorI& evictor, Int batchSize, bool loadServants) :
    _evictor(evictor),
    _batchSize(static_cast<size_t>(batchSize)),
    _loadServants(loadServants),
    _key(1024),
    _more(true)
{
    if(loadServants)
    {
	_value.resize(1024);
    }
    _batchIterator = _batch.end();
}


bool
Freeze::EvictorIteratorI::hasNext()
{
    if(_batchIterator != _batch.end()) 
    {
	return true;
    }
    else
    {
	_batchIterator = nextBatch();
	return (_batchIterator != _batch.end());
    }
}

Identity
Freeze::EvictorIteratorI::next()
{
    if(hasNext())
    {
	return *_batchIterator++;
    }
    else
    {
	throw Freeze::NoSuchElementException(__FILE__, __LINE__);
    }
}


vector<Identity>::const_iterator
Freeze::EvictorIteratorI::nextBatch()
{
    _batch.clear();

    if(!_more)
    {
	return _batch.end();
    }

    vector<EvictorI::EvictorElementPtr> evictorElements;
    evictorElements.reserve(_batchSize);
     
    Key firstKey;
    firstKey = _key;
   
    int loadedGeneration = 0;

    try
    {
	for(;;)
	{
	    _batch.clear();
	    evictorElements.clear();
	    
	    Dbt dbKey;
	    initializeOutDbt(_key, dbKey);

	    Dbt dbValue;
	    if(_loadServants)
	    {
		initializeOutDbt(_value, dbValue);
	    }
	    else
	    {
		dbValue.set_flags(DB_DBT_USERMEM | DB_DBT_PARTIAL);
	    }

	    Dbc* dbc = 0;
	    try
	    {
		//
		// Move to the first record
		// 
		u_int32_t flags = DB_NEXT;
		if(_key.size() > 0)
		{
		    //
		    // _key represents the next element not yet returned
		    // if it has been deleted, we want the one after
		    //
		    flags = DB_SET_RANGE;

		    //
		    // Will be used as input as well
		    //
		    dbKey.set_size(firstKey.size());
		}
		
		if(_loadServants)
		{
		    loadedGeneration = _evictor.currentGeneration();
		}

		_evictor.db()->cursor(0, &dbc, 0);
		
		for(;;)
		{
		    try
		    {
			_more = (dbc->get(&dbKey, &dbValue, flags) == 0);
			if(_more)
			{
			    _key.resize(dbKey.get_size());
			    //
			    // No need to resize data as we never use it as input
			    //
			}
			break;
		    }
		    catch(const DbMemoryException& dx)
		    {
			handleMemoryException(dx, _key, dbKey, _value, dbValue);
		    }
		}
		
		while(_batch.size() < _batchSize && _more)
		{
		    //
		    // Even when count is 0, we read one more record (unless we reach the end)
		    //
		    if(_loadServants)
		    {
			_more = _evictor.load(dbc, _key, _value, _batch, evictorElements);
		    }
		    else
		    {
			_more = _evictor.load(dbc, _key, _batch);
		    }
		}
		
		Dbc* toClose = dbc;
		dbc = 0;
		toClose->close();
		break; // for (;;)
	    }
	    catch(const DbDeadlockException&)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}
		_key = firstKey;
		//
		// Retry
		//
	    }
	    catch(...)
	    {
		if(dbc != 0)
		{
		    try
		    {
			dbc->close();
		    }
		    catch(const DbDeadlockException&)
		    {
			//
			// Ignored
			//
		    }
		}
		throw;
	    }
	}
    }
    catch(const DbException& dx)
    {
	DatabaseException ex(__FILE__, __LINE__);
	ex.message = dx.what();
	throw ex;
    }
    
    if(_batch.size() == 0)
    {
	return _batch.end();
    }
    else
    {
	if(_loadServants)
	{
	    _evictor.insert(_batch, evictorElements, loadedGeneration);
	}
	return _batch.begin();
    }
}
