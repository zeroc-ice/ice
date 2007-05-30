// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    public void 
    postCompletion(boolean committed, boolean deadlock)
    {    
        try
        {
            if(committed)
            {
                //
                // Remove updated & removed objects from cache
                //
                java.util.Iterator p = _invalidateList.iterator();
                while(p.hasNext())
                {
                    ToInvalidate ti = (ToInvalidate)p.next();
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

    public boolean
    response(boolean ok)
    {
        if(Thread.currentThread().equals(_owner))
        {
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

    public boolean
    exception(java.lang.Exception ex)
    {
        if(ex instanceof DeadlockException && Thread.currentThread().equals(_owner))
        {
            _deadlockException = (DeadlockException)ex;
            return false;
        }
        return true;
    }

   
    class ServantHolder
    {
        ServantHolder(Ice.Current current, ObjectStore store, boolean useNonmutating)
        {
            _current = current;
            _store = store;

            ServantHolder sh = findServantHolder(_current.id, _store);
            if(sh != null)
            {
                if(!sh._removed)
                {
                    _rec = sh._rec;
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
                    _stack.push(this);
                    _ownServant = true;
                    
                    //
                    // Compute readonly properly
                    //
                    _readOnly = (useNonmutating && current.mode == Ice.OperationMode.Nonmutating) ||
                        (!useNonmutating && (_rec.servant.ice_operationAttributes(current.operation) & 0x1) == 0);
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
                        EvictorI.updateStats(_rec.stats, System.currentTimeMillis());
                        _store.update(_current.id, _rec, _tx);
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

        private boolean _readOnly = false;
        private boolean _ownServant = false;
        private boolean _removed = false;

        private final Ice.Current _current;
        private final ObjectStore _store;
        private ObjectRecord _rec;
    };


    TransactionalEvictorContext(SharedDbEnv dbEnv)
    {
        _tx = (TransactionI)(new ConnectionI(dbEnv).beginTransaction());
        _owner = Thread.currentThread();
        _tx.adoptConnection();

        _tx.setPostCompletionCallback(this);
    }

    TransactionalEvictorContext(TransactionI tx)
    {
        _tx = tx;
        _owner = Thread.currentThread();

        _tx.setPostCompletionCallback(this);
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
    }
    
  
    TransactionI 
    transaction()
    {
        return _tx;
    }

    ServantHolder
    createServantHolder(Ice.Current current, ObjectStore store, boolean useNonmutating)
    {
        return new ServantHolder(current, store, useNonmutating);
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
    
 
    protected void 
    finalize()
    {
        if(_tx != null)
        {
            _tx.getConnectionI().communicator().getLogger().warning
                ("Finalizing incomplete TransactionalEvictorContext on DbEnv '" +  _tx.getConnectionI().dbEnv().getEnvName() + "'");
        }
    }

    private ServantHolder
    findServantHolder(Ice.Identity ident, ObjectStore store)
    {
        java.util.Iterator p = _stack.iterator();
        while(p.hasNext())
        {
            ServantHolder sh = (ServantHolder)p.next();
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


    //
    // Stack of ServantHolder
    //
    private final java.util.Stack _stack = new java.util.Stack();
    
    //
    // List of objects to invalidate from the caches upon commit
    //
    private final java.util.List _invalidateList = new java.util.LinkedList();

    private TransactionI _tx;
    private final Thread _owner;

    private DeadlockException _deadlockException;

    //
    // Protected by this
    //
    private boolean _deadlockExceptionDetected = false;
}
