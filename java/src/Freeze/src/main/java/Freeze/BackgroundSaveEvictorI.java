// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

//
// A Freeze evictor implementation that saves updates asynchronously,
// in a "saving" thread
//

class BackgroundSaveEvictorI extends EvictorI implements BackgroundSaveEvictor, Runnable
{
    //
    // Clean object; can become modified or destroyed
    //
    static final byte clean = 0;

    //
    // New object; can become clean, dead or destroyed
    //
    static final byte created = 1;

    //
    // Modified object; can become clean or destroyed
    //
    static final byte modified = 2;

    //
    // Being saved. Can become dead or created
    //
    static final byte destroyed = 3;

    //
    // Exists only in the SaveAsyncEvictor; for example the object was created
    // and later destroyed (without a save in between), or it was
    // destroyed on disk but is still in use. Can become created.
    //
    static final byte dead = 4;

    BackgroundSaveEvictorI(Ice.ObjectAdapter adapter, String envName, String filename,
                           ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        this(adapter, envName, null, filename, initializer, indices, createDb);
    }

    BackgroundSaveEvictorI(Ice.ObjectAdapter adapter, String envName, com.sleepycat.db.Environment dbEnv,
                           String filename, ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        super(adapter, envName, dbEnv, filename, null, initializer, indices, createDb);

        String propertyPrefix = "Freeze.Evictor." + envName + '.' + filename;

        //
        // By default, we save every minute or when the size of the modified
        // queue reaches 10.
        //

        _saveSizeTrigger =
            _communicator.getProperties().getPropertyAsIntWithDefault(propertyPrefix + ".SaveSizeTrigger", 10);

        _savePeriod =
            _communicator.getProperties().getPropertyAsIntWithDefault(propertyPrefix + ".SavePeriod", 60 * 1000);

        //
        // By default, we save at most 10 * SaveSizeTrigger objects per transaction
        //
        _maxTxSize = _communicator.getProperties().getPropertyAsIntWithDefault(
            propertyPrefix + ".MaxTxSize", 10 * _saveSizeTrigger);

        if(_maxTxSize <= 0)
        {
            _maxTxSize = 100;
        }

        //
        // By default, no stream timeout
        //
        _streamTimeout =
            _communicator.getProperties().getPropertyAsIntWithDefault(propertyPrefix + ".StreamTimeout", 0) * 1000;
        if(_streamTimeout > 0)
        {
            _timer = IceInternal.Util.getInstance(_communicator).timer();
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
        savingThreadName += "FreezeEvictorThread(" + envName + '.' + _filename + ")";
        _thread = new Thread(this, savingThreadName);
        _thread.start();
    }

    @Override
    public Ice.ObjectPrx
    addFacet(Ice.Object servant, Ice.Identity ident, String facet)
    {
        checkIdentity(ident);
        checkServant(servant);

        if(facet == null)
        {
            facet = "";
        }

        _deactivateController.lock();
        try
        {
            ObjectStore store = findStore(facet, _createDb);

            if(store == null)
            {
                NotFoundException ex = new NotFoundException();
                ex.message = _errorPrefix + "addFacet: could not open database for facet '" + facet + "'";
                throw ex;
            }

            boolean alreadyThere = false;

            for(;;)
            {
                //
                // Create a new entry
                //

                EvictorElement element = new EvictorElement(ident, store);
                element.status = dead;
                element.rec = new ObjectRecord();

                if(store.keepStats())
                {
                    element.rec.stats = new Statistics();
                }

                Object o = store.cache().putIfAbsent(ident, element);

                if(o != null)
                {
                    element = (EvictorElement)o;
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
                            case clean:
                            case created:
                            case modified:
                            {
                                alreadyThere = true;
                                break;
                            }
                            case destroyed:
                            {
                                element.status = modified;
                                element.rec.servant = servant;

                                //
                                // No need to push it on the modified queue, as a destroyed object
                                // is either already on the queue or about to be saved. When saved,
                                // it becomes dead.
                                //
                                break;
                            }
                            case dead:
                            {
                                element.status = created;
                                ObjectRecord rec = element.rec;

                                rec.servant = servant;

                                if(store.keepStats())
                                {
                                    rec.stats.creationTime = IceInternal.Time.currentMonotonicTimeMillis();
                                    rec.stats.lastSaveTime = 0;
                                    rec.stats.avgSaveTime = 0;
                                }

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
                ex.id = _communicator.identityToString(ident);
                if(facet.length() > 0)
                {
                    ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "");
                }
                throw ex;
            }

            if(_trace >= 1)
            {
                String objString = "object \"" + _communicator.identityToString(ident) + "\"";
                if(!facet.equals(""))
                {
                    objString += " with facet \"" + facet + "\"";
                }

                _communicator.getLogger().trace("Freeze.Evictor", "added " + objString + " to Db \"" + _filename +
                                                "\"");
            }

            Ice.ObjectPrx obj = _adapter.createProxy(ident);
            if(facet.length() > 0)
            {
                obj = obj.ice_facet(facet);
            }
            return obj;
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    public Ice.Object
    removeFacet(Ice.Identity ident, String facet)
    {
        checkIdentity(ident);
        if(facet == null)
        {
            facet = "";
        }

        _deactivateController.lock();

        try
        {
            ObjectStore store = findStore(facet, false);
            Ice.Object servant = null;

            if(store != null)
            {
                 for(;;)
                {
                    //
                    // Retrieve object
                    //
                    EvictorElement element = (EvictorElement)store.cache().pin(ident);
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
                                    case clean:
                                    {
                                        servant = element.rec.servant;
                                        element.status = destroyed;
                                        element.rec.servant = null;
                                        addToModifiedQueue(element);
                                        break;
                                    }
                                    case created:
                                    {
                                        servant = element.rec.servant;
                                        element.status = dead;
                                        element.rec.servant = null;
                                        break;
                                    }
                                    case modified:
                                    {
                                        servant = element.rec.servant;
                                        element.status = destroyed;
                                        element.rec.servant = null;
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
                                        break;
                                    }
                                    default:
                                    {
                                        assert false;
                                        break;
                                    }
                                }
                            }

                            if(element.keepCount > 0)
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
                ex.id = _communicator.identityToString(ident);
                if(facet.length() > 0)
                {
                    ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "");
                }
                throw ex;
            }

            if(_trace >= 1)
            {
                String objString = "object \"" + _communicator.identityToString(ident) + "\"";
                if(!facet.equals(""))
                {
                    objString += " with facet \"" + facet + "\"";
                }

                _communicator.getLogger().trace("Freeze.Evictor", "removed " + objString + " from Db \"" + _filename +
                                                "\"");
            }
            return servant;
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    public void
    keep(Ice.Identity ident)
    {
        keepFacet(ident, "");
    }

    @Override
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

            ObjectStore store = findStore(facet, false);
            if(store == null)
            {
                notThere = true;
            }
            else
            {
                for(;;)
                {
                    EvictorElement element = (EvictorElement)store.cache().pin(ident);
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
                            if(element.status == destroyed || element.status == dead)
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
                ex.id = _communicator.identityToString(ident);
                if(facet.length() > 0)
                {
                    ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "");
                }
                throw ex;
            }
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    public void
    release(Ice.Identity ident)
    {
        releaseFacet(ident, "");
    }

    @Override
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

            ObjectStore store = findStore(facet, false);

            if(store != null)
            {
                synchronized(this)
                {
                    EvictorElement element = (EvictorElement)store.cache().getIfPinned(ident);
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
            ex.id = _communicator.identityToString(ident);
            if(facet.length() > 0)
            {
                ex.id += " -f " + IceUtilInternal.StringUtil.escapeString(facet, "");
            }

            throw ex;
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    public boolean
    hasFacet(Ice.Identity ident, String facet)
    {
        checkIdentity(ident);
        if(facet == null)
        {
            facet = "";
        }

        _deactivateController.lock();
        try
        {

            ObjectStore store = findStore(facet, false);

            if(store == null)
            {
                return false;
            }

            synchronized(this)
            {
                EvictorElement element = (EvictorElement)store.cache().getIfPinned(ident);
                if(element != null)
                {
                    assert !element.stale;

                    synchronized(element)
                    {
                        return element.status != dead && element.status != destroyed;
                    }
                }
            }
            return store.dbHasObject(ident, null);
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    protected boolean
    hasAnotherFacet(Ice.Identity ident, String facet)
    {
        _deactivateController.lock();
        try
        {
            //
            // If the object exists in another store, throw FacetNotExistException
            // instead of returning null (== ObjectNotExistException)
            //
            java.util.Map<String, ObjectStore> storeMapCopy;
            synchronized(this)
            {
                storeMapCopy = new java.util.HashMap<String, ObjectStore>(_storeMap);
            }

            for(java.util.Map.Entry<String, ObjectStore> entry : storeMapCopy.entrySet())
            {
                //
                // Do not check facet
                //
                if(!facet.equals(entry.getKey()))
                {
                    ObjectStore store = entry.getValue();
                    boolean inCache = false;

                    synchronized(this)
                    {
                        EvictorElement element = (EvictorElement)store.cache().getIfPinned(ident);
                        if(element != null)
                        {
                            inCache = true;
                            assert !element.stale;

                            synchronized(element)
                            {
                                if(element.status != dead && element.status != destroyed)
                                {
                                    return true;
                                }
                            }
                        }
                    }
                    if(!inCache)
                    {
                        if(store.dbHasObject(ident, null))
                        {
                            return true;
                        }
                    }
                }
            }
            return false;
        }
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    protected Object
    createEvictorElement(Ice.Identity ident, ObjectRecord rec, ObjectStore store)
    {
        EvictorElement elt = new EvictorElement(ident, store);
        elt.rec = rec;
        return elt;
    }

    @Override
    protected Ice.Object
    locateImpl(Ice.Current current, Ice.LocalObjectHolder cookie)
    {
        _deactivateController.lock();
        try
        {
            cookie.value = null;

            ObjectStore store = findStore(current.facet, false);
            if(store == null)
            {
                if(_trace >= 2)
                {
                    _communicator.getLogger().trace("Freeze.Evictor", "locate could not find a database for facet \"" +
                                                    current.facet + "\"");
                }
                return null;
            }

            for(;;)
            {
                EvictorElement element = (EvictorElement)store.cache().pin(current.id);
                if(element == null)
                {
                    if(_trace >= 2)
                    {
                        _communicator.getLogger().trace("Freeze.Evictor", "locate could not find \"" +
                                                        _communicator.identityToString(current.id) + "\" in Db \"" +
                                                        _filename + "\"");
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
                        if(element.status == destroyed || element.status == dead)
                        {
                            if(_trace >= 2)
                            {
                                _communicator.getLogger().trace("Freeze.Evictor", "locate found \"" +
                                                                _communicator.identityToString(current.id) +
                                                                "\" in the cache for Db \"" + _filename +
                                                                "\" but it was dead or destroyed");
                            }
                            return null;
                        }

                        //
                        // It's a good one!
                        //

                        if(_trace >= 2)
                        {
                            _communicator.getLogger().trace("Freeze.Evictor", "locate found \"" +
                                                            _communicator.identityToString(current.id) + "\" in Db \"" +
                                                            _filename + "\"");
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
        finally
        {
            _deactivateController.unlock();
        }
    }

    @Override
    public void
    finished(Ice.Current current, Ice.Object servant, java.lang.Object cookie)
    {
        _deactivateController.lock();
        try
        {
            if(cookie != null)
            {
                EvictorElement element = (EvictorElement)cookie;

                boolean enqueue = false;

                if((servant.ice_operationAttributes(current.operation) & 0x1) != 0)
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

    @Override
    public void
    deactivate(String category)
    {
        if(_deactivateController.deactivate())
        {
            try
            {
                saveNow();

                synchronized(this)
                {
                    //
                    // Set the evictor size to zero, meaning that we will evict
                    // everything possible.
                    //
                    _evictorSize = 0;
                    evict();

                    _savingThreadDone = true;
                    notifyAll();
                }

                try
                {
                    _thread.join();
                }
                catch(InterruptedException ex)
                {
                }

                closeDbEnv();
            }
            finally
            {
                _deactivateController.deactivationComplete();
            }
        }
    }

    @Override
    public void
    run()
    {
        try
        {
            for(;;)
            {
                java.util.List<EvictorElement> allObjects;
                java.util.List<EvictorElement> deadObjects = new java.util.LinkedList<EvictorElement>();

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
                                long preSave = IceInternal.Time.currentMonotonicTimeMillis();
                                wait(_savePeriod);
                                if(IceInternal.Time.currentMonotonicTimeMillis() > preSave + _savePeriod)
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
                    _modifiedQueue = new java.util.ArrayList<EvictorElement>();
                }

                int size = allObjects.size();

                java.util.List<StreamedObject> streamedObjectQueue = new java.util.ArrayList<StreamedObject>();

                long streamStart = IceInternal.Time.currentMonotonicTimeMillis();

                //
                // Stream each element
                //
                for(int i = 0; i < size; i++)
                {
                    EvictorElement element = allObjects.get(i);

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
                                case created:
                                case modified:
                                {
                                    servant = element.rec.servant;
                                    break;
                                }
                                case destroyed:
                                {
                                    streamedObjectQueue.add(stream(element, streamStart));

                                    element.status = dead;
                                    deadObjects.add(element);
                                    break;
                                }
                                case dead:
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
                            java.util.concurrent.Future<?> future = null;
                            if(_timer != null)
                            {
                                //
                                // The timer is used to ensure the streaming of some object does not take more than
                                // timeout ms. We only measure the time necessary to acquire the lock on the object
                                // (servant), not the streaming itself.
                                //
                                future = _timer.schedule(new Runnable()
                                    {
                                        @Override
                                        public void run()
                                        {
                                            _communicator.getLogger().error(_errorPrefix +
                                                "Fatal error: streaming watch dog timed out");

                                            Util.handleFatalError(BackgroundSaveEvictorI.this, _communicator, null);
                                        }
                                    }, _streamTimeout, java.util.concurrent.TimeUnit.MILLISECONDS);
                            }
                            synchronized(servant)
                            {
                                if(future != null)
                                {
                                    future.cancel(false);
                                    future = null;
                                }

                                synchronized(element)
                                {
                                    byte status = element.status;

                                    switch(status)
                                    {
                                        case created:
                                        case modified:
                                        {
                                            if(servant == element.rec.servant)
                                            {
                                                streamedObjectQueue.add(stream(element, streamStart));

                                                element.status = clean;
                                            }
                                            else
                                            {
                                                tryAgain = true;
                                            }
                                            break;
                                        }
                                        case destroyed:
                                        {
                                            streamedObjectQueue.add(stream(element, streamStart));

                                            element.status = dead;
                                            deadObjects.add(element);
                                            break;
                                        }
                                        case dead:
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
                    long now = IceInternal.Time.currentMonotonicTimeMillis();
                    _communicator.getLogger().trace("Freeze.Evictor", "streamed " + streamedObjectQueue.size() +
                                                    " objects in " + (now - streamStart) + " ms");
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

                        long saveStart = 0;
                        if(_trace >= 1)
                        {
                            saveStart = IceInternal.Time.currentMonotonicTimeMillis();
                        }
                        String txnId = null;

                        try
                        {
                            com.sleepycat.db.Transaction tx = _dbEnv.getEnv().beginTransaction(null, null);

                            if(_txTrace >= 1)
                            {
                                txnId = Long.toHexString((tx.getId() & 0x7FFFFFFF) + 0x80000000L);

                                _communicator.getLogger().trace("Freeze.Evictor", _errorPrefix +
                                                                "started transaction " + txnId + " in saving thread");
                            }

                            try
                            {
                                for(int i = 0; i < txSize; i++)
                                {
                                    StreamedObject obj =  streamedObjectQueue.get(i);
                                    obj.store.save(obj.key, obj.value, obj.status, tx);
                                }

                                com.sleepycat.db.Transaction toCommit = tx;
                                tx = null;
                                toCommit.commit();

                                if(_txTrace >= 1)
                                {
                                    _communicator.getLogger().trace("Freeze.Evictor", _errorPrefix +
                                                                    "committed transaction " + txnId);
                                }
                            }
                            finally
                            {
                                if(tx != null)
                                {
                                    tx.abort();
                                    if(_txTrace >= 1)
                                    {
                                        _communicator.getLogger().trace("Freeze.Evictor", _errorPrefix +
                                                                        "rolled back transaction " + txnId);
                                    }
                                }
                            }

                            for(int i = 0; i < txSize; i++)
                            {
                                streamedObjectQueue.remove(0);
                            }

                            if(_trace >= 1)
                            {
                                long now = IceInternal.Time.currentMonotonicTimeMillis();
                                _communicator.getLogger().trace("Freeze.Evictor", "saved " + txSize + " objects in " +
                                                                (now - saveStart) + " ms");
                            }
                        }
                        catch(com.sleepycat.db.DeadlockException dx)
                        {
                            if(_deadlockWarning)
                            {
                                _communicator.getLogger().warning(
                                    "Deadlock in Freeze.BackgroundSaveEvictorI.run while writing " +
                                    "into Db \"" + _filename + "\"; retrying...");
                            }

                            tryAgain = true;
                            txSize = (txSize + 1)/2;
                        }
                        catch(com.sleepycat.db.DatabaseException dx)
                        {
                            throw new DatabaseException(_errorPrefix + "saving: " + dx.getMessage(), dx);
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
                        EvictorElement element = allObjects.get(i);
                        assert element.usageCount > 0;
                        element.usageCount--;
                    }
                    allObjects.clear();

                    for(EvictorElement element : deadObjects)
                    {
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
                                if(element.status == dead)
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
            _communicator.getLogger().error(_errorPrefix + "Fatal error in saving thread:\n" + sw.toString());

            Util.handleFatalError(this, _communicator, ex);
        }
    }

    @Override
    protected void
    evict()
    {
        assert Thread.holdsLock(this);

        java.util.Iterator<EvictorElement> p = _evictorList.riterator();
        while(p.hasNext() && _currentEvictorSize > _evictorSize)
        {
            //
            // Get the last unused element from the evictor queue.
            //
            EvictorElement element = p.next();
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
                    String objString = "object \"" + _communicator.identityToString(element.identity) + "\"";
                    String facet = element.store.facet();
                    if(facet.length() > 0)
                    {
                        objString += " with facet \"" + facet + "\"";
                    }

                    _communicator.getLogger().trace("Freeze.Evictor", "evicting " + objString + " from the queue; " +
                                                    "number of elements in the queue: " + _currentEvictorSize);
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

    @Override
    protected TransactionI
    beforeQuery()
    {
        saveNow();
        return null;
    }

    synchronized private void
    saveNow()
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
        }
        while(_saveNowThreads.contains(myself));
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

        assert element.status != dead;

        StreamedObject obj = new StreamedObject();

        obj.status = element.status;
        obj.store = element.store;
        obj.key = ObjectStore.marshalKey(element.identity, _communicator, _encoding);

        if(element.status != destroyed)
        {
            boolean keepStats = obj.store.keepStats();

            if(keepStats)
            {
                updateStats(element.rec.stats, streamStart);
            }
            obj.value = ObjectStore.marshalValue(element.rec, _communicator, _encoding, keepStats);
        }
        return obj;
    }

    static private class EvictorElement
    {
        EvictorElement(Ice.Identity identity, ObjectStore store)
        {
            this.identity = identity;
            this.store = store;
        }

        final ObjectStore store;
        final Ice.Identity identity;

        //
        // Protected by SaveAsyncEvictor
        //
        java.util.Iterator<EvictorElement> evictPosition = null;
        int usageCount = -1;
        int keepCount = 0;
        boolean stale = false;

        //
        // Protected by this
        //
        ObjectRecord rec = null;
        byte status = clean;
    }

    static private class StreamedObject
    {
        com.sleepycat.db.DatabaseEntry key = null;
        com.sleepycat.db.DatabaseEntry value = null;
        byte status = dead;
        ObjectStore store = null;
    }

    //
    // List of EvictorElement with stable iterators
    //
    private final Freeze.LinkedList<EvictorElement> _evictorList = new Freeze.LinkedList<EvictorElement>();
    private int _currentEvictorSize = 0;

    //
    // The _modifiedQueue contains a queue of all modified facets
    // Each element in the queue "owns" a usage count, to ensure the
    // elements containing them remain in the map.
    //
    private java.util.List<EvictorElement> _modifiedQueue = new java.util.ArrayList<EvictorElement>();

    private boolean _savingThreadDone = false;
    private java.util.concurrent.ScheduledExecutorService _timer;
    private long _streamTimeout;

    //
    // Threads that have requested a "saveNow" and are waiting for
    // its completion
    //
    private final java.util.List<Thread> _saveNowThreads = new java.util.ArrayList<Thread>();

    private int _saveSizeTrigger;
    private int _maxTxSize;
    private long _savePeriod;

    private Thread _thread;
}
