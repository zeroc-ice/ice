// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

class EvictorIteratorI implements EvictorIterator
{
    @Override
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

    @Override
    public Ice.Identity
    next()
    {
        if(hasNext())
        {
            return _batchIterator.next();
        }
        else
        {
            throw new NoSuchElementException();
        }
    }

    EvictorIteratorI(ObjectStore store, TransactionI tx, int batchSize)
    {
        _store = store;
        _more = (store != null);
        _batchSize = batchSize;
        _tx = tx;

        assert batchSize > 0;

        _key.setReuseBuffer(true);

        //
        // dlen is 0, so we should not retrieve any value
        //
        _value.setPartial(true);
    }

    private java.util.Iterator<Ice.Identity>
    nextBatch()
    {
        if(!_more)
        {
            return null;
        }

        EvictorI.DeactivateController deactivateController = _store.evictor().deactivateController();
        deactivateController.lock();

        com.sleepycat.db.Transaction txn = _tx == null ? null : _tx.dbTxn();

        try
        {
            Ice.Communicator communicator = _store.communicator();
            Ice.EncodingVersion encoding = _store.encoding();

            byte[] firstKey = null;
            if(_key.getSize() > 0)
            {
                firstKey = new byte[_key.getSize()];
                System.arraycopy(_key.getData(), 0, firstKey, 0, firstKey.length);
            }

            for(;;)
            {
                com.sleepycat.db.Cursor dbc = null;

                _batch = new java.util.ArrayList<Ice.Identity>();

                try
                {
                    //
                    // Move to the first record
                    //
                    boolean range = false;
                    if(firstKey != null)
                    {
                        //
                        // _key represents the next element not yet returned
                        // if it has been deleted, we want the one after
                        //
                        range = true;
                    }

                    dbc = _store.db().openCursor(txn, null);

                    boolean done = false;
                    do
                    {
                        com.sleepycat.db.OperationStatus status;
                        if(range)
                        {
                            status = dbc.getSearchKeyRange(_key, _value, null);
                        }
                        else
                        {
                            status = dbc.getNext(_key, _value, null);
                        }
                        _more = (status == com.sleepycat.db.OperationStatus.SUCCESS);

                        if(_more)
                        {
                            range = false;

                            if(_batch.size() < _batchSize)
                            {
                                Ice.Identity ident = ObjectStore.unmarshalKey(_key, communicator, encoding);
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
                catch(com.sleepycat.db.DeadlockException dx)
                {
                    if(_store.evictor().deadlockWarning())
                    {
                        communicator.getLogger().warning("Deadlock in Freeze.EvictorIteratorI.load while " +
                                                         "iterating over Db \"" + _store.evictor().filename() + "/" +
                                                         _store.dbName() + "\"");
                    }

                    if(_tx == null)
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

                        //
                        // Retry
                        //
                    }
                    else
                    {
                        throw new DeadlockException(_store.evictor().errorPrefix() + "Db.cursor: " + dx.getMessage(),
                                                    _tx, dx);
                    }
                }
                catch(com.sleepycat.db.DatabaseException dx)
                {
                    throw new DatabaseException(_store.evictor().errorPrefix() + "Db.cursor: " + dx.getMessage(), dx);
                }
                finally
                {
                    if(dbc != null)
                    {
                        try
                        {
                            dbc.close();
                        }
                        catch(com.sleepycat.db.DeadlockException dx)
                        {
                            if(_tx != null)
                            {
                                throw new DeadlockException(
                                    _store.evictor().errorPrefix() + "Db.cursor: " + dx.getMessage(), _tx, dx);
                            }
                        }
                        catch(com.sleepycat.db.DatabaseException dx)
                        {
                            //
                            // Ignored
                            //
                        }
                    }
                }
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
    private final TransactionI _tx;
    private final int _batchSize;
    private java.util.Iterator<Ice.Identity> _batchIterator;

    private final com.sleepycat.db.DatabaseEntry _key = new com.sleepycat.db.DatabaseEntry();
    private final com.sleepycat.db.DatabaseEntry _value = new com.sleepycat.db.DatabaseEntry();
    private java.util.List<Ice.Identity> _batch = null;
    private boolean _more = true;
}
