// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

//
// A Freeze transactional evictor context holds a Berkeley DB transaction
//  and a number of servants loaded using this transaction.
//

class TransactionalEvictorContext implements Ice.DispatchInterceptorAsyncCallback, PostCompletionCallback
{
    @Override
    public void
    postCompletion(boolean committed, boolean deadlock, SharedDbEnv dbEnv)
    {
        //
        // Clear the current context
        //
        dbEnv.setCurrentTransaction(null);

        try
        {
            if(committed)
            {
                //
                // Remove updated & removed objects from cache
                //
                for(ToInvalidate ti : _invalidateList)
                {
                    ti.invalidate();
                }
                _invalidateList.clear();
            }
        }
        finally
        {
            synchronized(this)
            {
                if(_tx != null)
                {
                    if(deadlock)
                    {
                        _deadlockExceptionDetected = true;
                    }
                    _tx = null;
                    notifyAll();
                }
            }
        }
    }

    @Override
    public boolean
    response(boolean ok)
    {
        if(Thread.currentThread().equals(_owner))
        {
            if(!ok)
            {
                _userExceptionDetected = true;
            }

            return true;
        }
        else
        {
            synchronized(this)
            {
                if(_deadlockExceptionDetected)
                {
                    return false;
                }
                if(_tx == null)
                {
                    return true;
                }
                try
                {
                    wait();
                }
                catch(InterruptedException ex)
                {
                }
                return !_deadlockExceptionDetected;
            }
        }
    }

    @Override
    public boolean
    exception(java.lang.Exception ex)
    {
        if(ex instanceof DeadlockException && Thread.currentThread().equals(_owner))
        {
            _deadlockException = (DeadlockException)ex;
            return false;
        }

        if(ex instanceof TransactionalEvictorDeadlockException && Thread.currentThread().equals(_owner))
        {
            _nestedCallDeadlockException = (TransactionalEvictorDeadlockException)ex;
            return false;
        }

        return true;
    }

    class ServantHolder
    {
        ServantHolder(Ice.Current current, ObjectStore store)
        {
            _current = current;
            _store = store;

            ServantHolder sh = findServantHolder(_current.id, _store);
            if(sh != null)
            {
                if(!sh._removed)
                {
                    _rec = sh._rec;
                    _readOnly = sh._readOnly;

                    if(_trace >= 3)
                    {
                        _communicator.getLogger().trace("Freeze.Evictor", "found \"" +
                                                        _communicator.identityToString(_current.id) +
                                                        "\" with facet \"" + _store.facet() + "\" in current context");
                    }
                }
            }
            else
            {
                //
                // Let's load this servant
                //
                _rec = store.load(current.id, _tx);
                if(_rec != null)
                {
                    if(_trace >= 3)
                    {
                        _communicator.getLogger().trace("Freeze.Evictor", "loaded \"" +
                                                        _communicator.identityToString(_current.id) +
                                                        "\" with facet \"" + _store.facet() +
                                                        "\" into current context");
                    }

                    _stack.push(this);
                    _ownServant = true;
                }
            }
        }

        void
        markReadWrite()
        {
            if(_ownServant)
            {
                _readOnly = false;
            }
            else
            {
                if(_readOnly)
                {
                    throw new DatabaseException("freeze:write operation called from freeze:read operation");
                }
            }
        }

        void
        release()
        {
            if(_ownServant)
            {
                if(_tx != null)
                {
                    if(!_readOnly && !_removed)
                    {
                        if(_store.keepStats())
                        {
                            EvictorI.updateStats(_rec.stats, IceInternal.Time.currentMonotonicTimeMillis());
                        }
                        _store.update(_current.id, _rec, _tx);

                        if(_trace >= 3)
                        {
                            _communicator.getLogger().trace("Freeze.Evictor", "updated \"" +
                                                            _communicator.identityToString(_current.id) +
                                                            "\" with facet \"" + _store.facet() +
                                                            "\" within transaction");
                        }
                    }

                    if(!_readOnly || _removed)
                    {
                        _invalidateList.add(new ToInvalidate(_current.id, _store));
                    }
                }
                _stack.pop();
            }
        }

        boolean
        matches(Ice.Identity ident, ObjectStore store)
        {
            return ident.equals(_current.id) && store == _store;
        }

        Ice.Object
        servant()
        {
            if(_rec == null)
            {
                return null;
            }
            else
            {
                return _rec.servant;
            }
        }

        void
        removed()
        {
            _removed = true;
        }

        private boolean _ownServant = false;
        private boolean _removed = false;
        private boolean _readOnly = true;

        private final Ice.Current _current;
        private final ObjectStore _store;
        private ObjectRecord _rec;
    }

    TransactionalEvictorContext(SharedDbEnv dbEnv)
    {
        _communicator = dbEnv.getCommunicator();
        _trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Evictor");
        _tx = (TransactionI)(new ConnectionI(dbEnv).beginTransaction());
        _owner = Thread.currentThread();
        _tx.adoptConnection();

        _tx.setPostCompletionCallback(this);
    }

    TransactionalEvictorContext(TransactionI tx, Ice.Communicator communicator)
    {
        _communicator = communicator;
        _trace = _communicator.getProperties().getPropertyAsInt("Freeze.Trace.Evictor");
        _tx = tx;
        _owner = Thread.currentThread();

        _tx.setPostCompletionCallback(this);
    }

    Ice.Object
    findServant(Ice.Identity ident, ObjectStore store)
    {
        ServantHolder sh = findServantHolder(ident, store);
        return sh != null ? sh.servant() : null;
    }

    void
    rollback()
    {
        if(_tx != null)
        {
            _tx.rollback();
        }
    }

    void
    commit()
    {
        if(_tx != null)
        {
            _tx.commit();
        }
    }

    void
    checkDeadlockException()
    {
        if(_deadlockException != null)
        {
            throw _deadlockException;
        }

        if(_nestedCallDeadlockException != null)
        {
            throw _nestedCallDeadlockException;
        }
    }

    boolean
    clearUserException()
    {
        //
        // No need to synchronize; _userExceptionDetected is only read/written by the
        // dispatch thread
        //
        boolean result = _userExceptionDetected;
        _userExceptionDetected = false;
        return result;
    }

    TransactionI
    transaction()
    {
        return _tx;
    }

    ServantHolder
    createServantHolder(Ice.Current current, ObjectStore store)
    {
        return new ServantHolder(current, store);
    }

    void
    deadlockException()
    {
        synchronized(this)
        {
            _deadlockExceptionDetected = true;
            notifyAll();
        }

        rollback();
    }

    Ice.Object
    servantRemoved(Ice.Identity ident, ObjectStore store)
    {
        if(_tx != null)
        {
            //
            // Lookup servant holder on stack
            //
            ServantHolder sh = findServantHolder(ident, store);
            if(sh != null)
            {
                sh.removed();
                return sh.servant();
            }
            else
            {
                _invalidateList.add(new ToInvalidate(ident, store));
                return null;
            }
        }
        return null;
    }

    private ServantHolder
    findServantHolder(Ice.Identity ident, ObjectStore store)
    {
        for(ServantHolder sh : _stack)
        {
            if(sh.matches(ident, store))
            {
                return sh;
            }
        }
        return null;
    }

    private static class ToInvalidate
    {
        ToInvalidate(Ice.Identity ident, ObjectStore store)
        {
            _ident = ident;
            _store = store;
        }

        void
        invalidate()
        {
            ((TransactionalEvictorI)_store.evictor()).evict(_ident, _store);
        }

        private final Ice.Identity _ident;
        private final ObjectStore _store;
    }

    private final java.util.Stack<ServantHolder> _stack = new java.util.Stack<ServantHolder>();

    //
    // List of objects to invalidate from the caches upon commit
    //
    private final java.util.List<ToInvalidate> _invalidateList = new java.util.LinkedList<ToInvalidate>();

    private TransactionI _tx;
    private final Thread _owner;

    private DeadlockException _deadlockException;
    private TransactionalEvictorDeadlockException _nestedCallDeadlockException;

    //
    // Protected by this
    //
    private boolean _deadlockExceptionDetected = false;

    //
    // Only updated by the dispatch thread
    //
    private boolean _userExceptionDetected = false;

    private final int _trace;
    private final Ice.Communicator _communicator;
}
