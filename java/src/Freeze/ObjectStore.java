// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class ObjectStore implements IceUtil.Store
{
    
    ObjectStore(String facet, boolean createDb, EvictorI evictor, 
		java.util.List indices, boolean populateEmptyIndices)
    {
	_cache = new IceUtil.Cache(this);
	
	_facet = facet;
	_evictor = evictor;
	_indices = indices;
	_communicator = evictor.communicator();
	
	if(facet.equals(""))
	{
	    _dbName = EvictorI.defaultDb;
	}
	else
	{
	    _dbName = facet;
	}

	Connection connection = Util.createConnection(_communicator, evictor.dbEnv().getEnvName());

	try
	{
	    Catalog catalog = new Catalog(connection, Util.catalogName(), true);	    
	    CatalogData catalogData = (CatalogData)catalog.get(evictor.filename());
	    
	    if(catalogData != null && catalogData.evictor == false)
	    {
		DatabaseException ex = new DatabaseException();
		ex.message = _evictor.errorPrefix() + evictor.filename() + " is not an evictor database";
		throw ex;
	    }
	    
	    com.sleepycat.db.DbEnv dbEnv = evictor.dbEnv().getEnv();
	    
	    try
	    {	    
		_db = new com.sleepycat.db.Db(dbEnv, 0);
		
		Transaction tx = connection.beginTransaction();
		com.sleepycat.db.DbTxn txn = Util.getTxn(tx);

		//
		// TODO: FREEZE_DB_MODE
		//
		int flags = 0;
		if(createDb)
		{
		    flags |= com.sleepycat.db.Db.DB_CREATE;
		}
		_db.open(txn, evictor.filename(), _dbName, com.sleepycat.db.Db.DB_BTREE, flags, 0);
		
		
		java.util.Iterator p = _indices.iterator();
		while(p.hasNext())
		{
		    Index index = (Index) p.next();
		    index.associate(this, txn, createDb, populateEmptyIndices);
		}
		
		if(catalogData == null)
		{
		    catalogData = new CatalogData();
		    catalogData.evictor = true;
		    catalog.put(evictor.filename(), catalogData);
		}
		
		tx.commit();
	    }
	    catch(java.io.FileNotFoundException dx)
	    {
		NotFoundException ex = new NotFoundException();
		ex.initCause(dx);
		ex.message = _evictor.errorPrefix() + "Db.open: " + dx.getMessage();
		throw ex;
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _evictor.errorPrefix() + "Db.open: " + dx.getMessage();
		throw ex;
	    }
	    finally
	    {
		Transaction tx = connection.currentTransaction();
		if(tx != null)
		{
		    try
		    {
			tx.rollback();
		    }
		    catch(DatabaseException de)
		    {
		    }
		}
	    }
	}
	finally
	{
	    connection.close();
	}
    }

    protected void
    finalize()
    {
        if(_db != null)
        {
            close();
        }
    }
    
    void
    close()
    {
	try
	{
	    _db.close(0);
	   
	    java.util.Iterator p = _indices.iterator();
	    while(p.hasNext())
	    {
		Index index = (Index) p.next();
		index.close();
	    }
	    _indices.clear();
	}
	catch(com.sleepycat.db.DbException dx)
	{
	    DatabaseException ex = new DatabaseException();
	    ex.initCause(dx);
	    ex.message = _evictor.errorPrefix() + "Db.close: " + dx.getMessage();
	    throw ex;
	}
	_db = null;
    }
    
    boolean
    dbHasObject(Ice.Identity ident)
    {
	byte[] key = marshalKey(ident, _communicator);
	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
		
	//
	// Keep 0 length since we're not interested in the data
	//
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();
	dbValue.setFlags(com.sleepycat.db.Db.DB_DBT_PARTIAL);
	
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
		    throw new DatabaseException();
		}
	    }
	    catch(com.sleepycat.db.DbDeadlockException deadlock)
	    {
		if(_evictor.deadlockWarning())
		{
		    _communicator.getLogger().warning
			("Deadlock in Freeze.ObjectStore.dhHasObject while reading Db \"" 
			 + _evictor.filename() + "/" + _dbName 
			 + "\"; retrying ...");
		}

		//
		// Ignored, try again
		//
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _evictor.errorPrefix() + "Db.get: " + dx.getMessage();
		throw ex;
	    }
	}
    }

    void
    save(byte[] key, byte[] value, byte status, com.sleepycat.db.DbTxn tx)
	throws com.sleepycat.db.DbException
    {
	switch(status)
	{
	    case EvictorElement.created:
	    case EvictorElement.modified:
	    {
		com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
		com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt(value);
		int flags = (status == EvictorElement.created) ? com.sleepycat.db.Db.DB_NOOVERWRITE : 0;
		int err = _db.put(tx, dbKey, dbValue, flags);
		if(err != 0)
		{
		    throw new DatabaseException();
		}
		break;
	    }
	    case EvictorElement.destroyed:
	    {
		com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
		int err = _db.delete(tx, dbKey, 0);
		if(err != 0)
		{
		    throw new DatabaseException();
		}
		break;
	    }
	    default:
	    {
		assert false;
	    }
	}
    }

    static byte[]
    marshalKey(Ice.Identity v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
	v.__write(os);
	java.nio.ByteBuffer buf = os.prepareWrite();
	byte[] r = new byte[buf.limit()];
	buf.get(r);
	return r;
    }

    static Ice.Identity
    unmarshalKey(byte[] b, Ice.Communicator communicator)
    {
        IceInternal.BasicStream is = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
	is.resize(b.length, true);
	java.nio.ByteBuffer buf = is.prepareRead();
	buf.position(0);
	buf.put(b);
	buf.position(0);
	Ice.Identity key = new Ice.Identity();
	key.__read(is);
	return key;
    }

    static byte[]
    marshalValue(ObjectRecord v, Ice.Communicator communicator)
    {
        IceInternal.BasicStream os = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
	os.startWriteEncaps();
	v.__write(os);
	os.writePendingObjects();
	os.endWriteEncaps();
	java.nio.ByteBuffer buf = os.prepareWrite();
	byte[] r = new byte[buf.limit()];
	buf.get(r);
	return r;
    }

    static ObjectRecord
    unmarshalValue(byte[] b, Ice.Communicator communicator)
    {
        IceInternal.BasicStream is = new IceInternal.BasicStream(Ice.Util.getInstance(communicator));
        is.sliceObjects(false);
	is.resize(b.length, true);
	java.nio.ByteBuffer buf = is.prepareRead();
	buf.position(0);
	buf.put(b);
	buf.position(0);
	ObjectRecord rec= new ObjectRecord();
	is.startReadEncaps();
	rec.__read(is);
	is.readPendingObjects();
	is.endReadEncaps();
	return rec;
    }


    final IceUtil.Cache
    cache()
    {
	return _cache;
    }

    final com.sleepycat.db.Db 
    db()
    {
	return _db;
    }
    
    final Ice.Communicator
    communicator()
    {
	return _communicator;
    }
    
    final EvictorI
    evictor()
    {
	return _evictor;
    }
    
    final String
    facet()
    {
	return _facet;
    }

    final String
    dbName()
    {
	return _dbName;
    }

    public Object
    load(Object identObj)
    {
	Ice.Identity ident = (Ice.Identity) identObj;

	byte[] key = marshalKey(ident, _communicator);

	com.sleepycat.db.Dbt dbKey = new com.sleepycat.db.Dbt(key);
	com.sleepycat.db.Dbt dbValue = new com.sleepycat.db.Dbt();

	for(;;)
	{
	    try
	    {	
		int rs = _db.get(null, dbKey, dbValue, 0);
		
		if(rs == com.sleepycat.db.Db.DB_NOTFOUND)
		{
		    return null;
		}
		else if (rs != 0)
		{
		    assert false;
		    throw new DatabaseException();
		}
		break;
	    }
	    catch(com.sleepycat.db.DbDeadlockException deadlock)
	    {
		if(_evictor.deadlockWarning())
		{
		    _communicator.getLogger().warning
			("Deadlock in Freeze.ObjectStore.load while reading Db \"" 
			 + _evictor.filename() + "/" + _dbName 
			 + "\"; retrying ...");
		}

		//
		// Ignored, try again
		//
	    }
	    catch(com.sleepycat.db.DbException dx)
	    {
		DatabaseException ex = new DatabaseException();
		ex.initCause(dx);
		ex.message = _evictor.errorPrefix() + "Db.get: " + dx.getMessage();
		throw ex;
	    }
	}
	
	EvictorElement result = new EvictorElement(ident, this);
	result.rec = unmarshalValue(dbValue.getData(), _communicator);
	
	_evictor.initialize(ident, _facet, result.rec.servant);
	return result;
    }

    private final IceUtil.Cache _cache;
    
    private com.sleepycat.db.Db _db;
    private final String _facet;
    private final String _dbName;
    private final EvictorI _evictor;
    private final java.util.List _indices;
    private final Ice.Communicator _communicator;

}







