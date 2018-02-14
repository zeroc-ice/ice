// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

abstract class EvictorI implements Evictor
{
    //
    // The deactivate controller is used by the implementation of all public
    // operations to ensure that deactivate() (which closes/clears various
    // Berkeley DB objects) is not called during these operations.
    // Note that the only threads that may perform such concurrent calls
    // are threads other than the dispatch threads of the associated adapter.
    //
    class DeactivateController
    {
        synchronized void
        activate()
        {
            assert !_activated;
            _activated = true;
        }

        synchronized void
        lock()
        {
            assert _activated;

            if(_deactivated || _deactivating)
            {
                throw new EvictorDeactivatedException();
            }
            _guardCount++;
        }

        synchronized void
        unlock()
        {
            assert _activated;

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

        synchronized boolean
        deactivated()
        {
            return !_activated || _deactivated;
        }

        synchronized boolean
        deactivate()
        {
            assert _activated;

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
                        _communicator.getLogger().trace("Freeze.Evictor",  "Waiting for " + _guardCount +
                            " threads to complete before starting deactivation.");
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

        synchronized void
        deactivationComplete()
        {
            if(_trace >= 1)
            {
                _communicator.getLogger().trace("Freeze.Evictor", "Deactivation complete.");
            }

            _deactivated = true;
            _deactivating = false;
            notifyAll();
        }

        private boolean _activated = false;
        private boolean _deactivating = false;
        private boolean _deactivated = false;
        private int _guardCount = 0;
    }

    static final String defaultDb = "$default";
    static final String indexPrefix = "$index:";

    @Override
    public Ice.ObjectPrx
    add(Ice.Object servant, Ice.Identity ident)
    {
        return addFacet(servant, ident, "");
    }

    @Override
    public Ice.Object
    remove(Ice.Identity ident)
    {
        return removeFacet(ident, "");
    }

    @Override
    public boolean
    hasObject(Ice.Identity ident)
    {
        return hasFacet(ident, "");
    }

    @Override
    public Ice.Object
    locate(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
        //
        // Special ice_ping() handling
        //
        if(current.operation != null && current.operation.equals("ice_ping"))
        {
            if(hasFacet(current.id, current.facet))
            {
                if(_trace >= 3)
                {
                    _communicator.getLogger().trace(
                        "Freeze.Evictor", "ice_ping found \"" + _communicator.identityToString(current.id) +
                        "\" with facet \"" + current.facet + "\"");
                }

                cookie.value = null;
                return _pingObject;
            }
            else if(hasAnotherFacet(current.id, current.facet))
            {
                if(_trace >= 3)
                {
                    _communicator.getLogger().trace(
                        "Freeze.Evictor", "ice_ping raises FacetNotExistException for \"" +
                        _communicator.identityToString(current.id)  + "\" with facet \"" + current.facet + "\"");
                }

                throw new Ice.FacetNotExistException();
            }
            else
            {
                if(_trace >= 3)
                {
                    _communicator.getLogger().trace(
                        "Freeze.Evictor", "ice_ping will raise ObjectNotExistException for \"" +
                        _communicator.identityToString(current.id)  + "\" with facet \"" + current.facet + "\"");
                }

                return null;
            }
        }

        Ice.Object result = locateImpl(current, cookie);

        if(result == null)
        {
            if(hasAnotherFacet(current.id, current.facet))
            {
                throw new Ice.FacetNotExistException(current.id, current.facet, current.operation);
            }
        }
        return result;
    }

    @Override
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

    @Override
    synchronized public int
    getSize()
    {
        return _evictorSize;
    }

    @Override
    public EvictorIterator
    getIterator(String facet, int batchSize)
    {
        _deactivateController.lock();
        try
        {
            if(facet == null)
            {
                facet = "";
            }
            TransactionI tx = beforeQuery();
            return new EvictorIteratorI(findStore(facet, false), tx, batchSize);
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    abstract protected boolean hasAnotherFacet(Ice.Identity ident, String facet);

    abstract protected Object createEvictorElement(Ice.Identity ident, ObjectRecord rec, ObjectStore store);

    abstract protected Ice.Object locateImpl(Ice.Current current, Ice.LocalObjectHolder cookie);

    abstract protected void evict();

    protected void
    closeDbEnv()
    {
        assert _dbEnv != null;
        for(ObjectStore store : _storeMap.values())
        {
            store.close();
        }
        _dbEnv.close();
        _dbEnv = null;
    }

    protected synchronized ObjectStore
    findStore(String facet, boolean createIt)
    {
        ObjectStore os = _storeMap.get(facet);

        if(os == null && createIt)
        {
            String facetType = _facetTypes.get(facet);
            os = new ObjectStore(facet, facetType, true, this, new java.util.LinkedList<Index>(), false);
            _storeMap.put(facet, os);
        }
        return os;
    }

    protected void
    initialize(Ice.Identity ident, String facet, Ice.Object servant)
    {
        if(_initializer != null)
        {
            _initializer.initialize(_adapter, ident, facet, servant);
        }
    }

    protected
    EvictorI(Ice.ObjectAdapter adapter, String envName, com.sleepycat.db.Environment dbEnv, String filename,
             java.util.Map<String, String> facetTypes, ServantInitializer initializer, Index[] indices,
             boolean createDb)
    {
        _adapter = adapter;
        _communicator = adapter.getCommunicator();
        _initializer = initializer;
        _filename = filename;
        _createDb = createDb;
        _facetTypes = facetTypes == null ? new java.util.HashMap<String, String>() :
            new java.util.HashMap<String, String>(facetTypes);

        _dbEnv = SharedDbEnv.get(_communicator, envName, dbEnv);
        _encoding = _dbEnv.getEncoding();

        _trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Evictor");
        _txTrace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Transaction");
        _deadlockWarning = _communicator.getProperties().getPropertyAsInt("Freeze.Warn.Deadlocks") > 0;

        _errorPrefix = "Freeze Evictor DbEnv(\"" + envName + "\") Db(\"" + _filename + "\"): ";

        String propertyPrefix = "Freeze.Evictor." + envName + '.' + _filename;

        boolean populateEmptyIndices =
            _communicator.getProperties().getPropertyAsIntWithDefault(propertyPrefix + ".PopulateEmptyIndices", 0) > 0;

        //
        // Instantiate all Dbs in 2 steps:
        // (1) iterate over the indices and create ObjectStore with indices
        // (2) open ObjectStores without indices
        //

        java.util.List<String> dbs = allDbs();
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
                    java.util.List<Index> storeIndices = new java.util.LinkedList<Index>();
                    for(int j = i; j < indices.length; ++j)
                    {
                        if(indices[j].facet().equals(facet))
                        {
                            storeIndices.add(indices[j]);
                        }
                    }

                    String facetType = _facetTypes.get(facet);
                    ObjectStore store = new ObjectStore(facet, facetType,_createDb, this, storeIndices,
                                                        populateEmptyIndices);
                    _storeMap.put(facet, store);
                }
            }
        }

        for(String facet : dbs)
        {
            if(facet.equals(defaultDb))
            {
                facet = "";
            }

            if(_storeMap.get(facet) == null)
            {
                String facetType = _facetTypes.get(facet);

                ObjectStore store = new ObjectStore(facet, facetType, _createDb, this,
                                                    new java.util.LinkedList<Index>(), populateEmptyIndices);

                _storeMap.put(facet, store);
            }
        }
        _deactivateController.activate();
    }

    protected
    EvictorI(Ice.ObjectAdapter adapter, String envName, String filename, java.util.Map<String, String> facetTypes,
             ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        this(adapter, envName, null, filename, facetTypes, initializer, indices, createDb);
    }

    abstract TransactionI beforeQuery();

    static void
    updateStats(Statistics stats, long time)
    {
        long diff = time - (stats.creationTime + stats.lastSaveTime);
        if(stats.lastSaveTime == 0)
        {
            stats.lastSaveTime = diff;
            stats.avgSaveTime = diff;
        }
        else
        {
            stats.lastSaveTime = time - stats.creationTime;
            stats.avgSaveTime = (long)(stats.avgSaveTime * 0.95 + diff * 0.05);
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

    final Ice.EncodingVersion
    encoding()
    {
        return _encoding;
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

    final boolean
    deadlockWarning()
    {
        return _deadlockWarning;
    }

    final int
    trace()
    {
        return _trace;
    }

    private java.util.List<String>
    allDbs()
    {
        java.util.List<String> result = new java.util.LinkedList<String>();

        com.sleepycat.db.Database db = null;
        com.sleepycat.db.Cursor dbc = null;

        try
        {
            com.sleepycat.db.DatabaseConfig config = new com.sleepycat.db.DatabaseConfig();
            config.setType(com.sleepycat.db.DatabaseType.UNKNOWN);
            config.setReadOnly(true);
            db = _dbEnv.getEnv().openDatabase(null, _filename, null, config);

            dbc = db.openCursor(null, null);

            com.sleepycat.db.DatabaseEntry key = new com.sleepycat.db.DatabaseEntry();
            com.sleepycat.db.DatabaseEntry value = new com.sleepycat.db.DatabaseEntry();

            boolean more = true;
            while(more)
            {
                more = (dbc.getNext(key, value, null) == com.sleepycat.db.OperationStatus.SUCCESS);
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
            db.close();
            db = null;
        }
        catch(java.io.UnsupportedEncodingException ix)
        {
            throw new DatabaseException(_errorPrefix + "cannot decode database names", ix);
        }
        catch(java.io.FileNotFoundException ix)
        {
            //
            // New file
            //
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(_errorPrefix + "Db.open: " + dx.getMessage(), dx);
        }
        finally
        {
            if(dbc != null)
            {
                try
                {
                    dbc.close();
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    // Ignored
                }
            }

            if(db != null)
            {
                try
                {
                    db.close();
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    // Ignored
                }
            }
        }

        return result;
    }

    static void
    checkIdentity(Ice.Identity ident)
    {
        if(ident.name == null || ident.name.length() == 0)
        {
            throw new Ice.IllegalIdentityException(ident);
        }
    }

    static void
    checkServant(Ice.Object servant)
    {
        if(servant == null)
        {
            throw new Ice.IllegalServantException("cannot add null servant to Freeze Evictor");
        }
    }

    protected int _evictorSize = 10;

    protected final java.util.Map<String, ObjectStore> _storeMap = new java.util.HashMap<String, ObjectStore>();
    private final java.util.Map<String, String> _facetTypes;

    protected final Ice.ObjectAdapter _adapter;
    protected final Ice.Communicator _communicator;
    protected final Ice.EncodingVersion _encoding;

    protected final ServantInitializer _initializer;

    protected SharedDbEnv  _dbEnv;

    protected final String _filename;
    protected final boolean _createDb;

    protected int _trace = 0;
    protected int _txTrace = 0;

    protected String _errorPrefix;

    protected boolean _deadlockWarning;

    protected DeactivateController _deactivateController = new DeactivateController();

    private Ice.Object _pingObject = new PingObject();
}
