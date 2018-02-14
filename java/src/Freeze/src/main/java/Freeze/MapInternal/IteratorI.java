// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

import Freeze.DatabaseException;
import Freeze.DeadlockException;
import Freeze.NotFoundException;
import java.nio.ByteBuffer;

class IteratorI<K, V> implements Freeze.Map.EntryIterator<java.util.Map.Entry<K, V>>
{
    IteratorI(MapI<K, V> map, IteratorModel<K, V> model)
    {
        _map = map;
        _model = model;
        _trace = model.traceLevels();
        _dbName = model.dbName();

        try
        {
            _txn = _map.connection().dbTxn();
            _cursor = _model.openCursor();
        }
        catch(com.sleepycat.db.DeadlockException dx)
        {
            dead();
            throw new DeadlockException(_trace.errorPrefix + "EntryIterator constructor: " + dx.getMessage(),
                                        _map.connection().currentTransaction(), dx);
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            dead();
            throw new DatabaseException(_trace.errorPrefix + "EntryIterator constructor: " + dx.getMessage(), dx);
        }

        _iteratorListToken = _map.addIterator(this);
    }

    @Override
    public boolean
    hasNext()
    {
        if(_current == null || _current == _lastReturned)
        {
            try
            {
                if(_current == null)
                {
                    _current = _model.firstEntry(_cursor);
                }
                else
                {
                    _current = _model.nextEntry(_cursor);
                }
            }
            catch(com.sleepycat.db.DeadlockException dx)
            {
                dead();
                throw new DeadlockException(_trace.errorPrefix + "Dbc.get: " + dx.getMessage(),
                                            _map.connection().currentTransaction(), dx);
            }
            catch(com.sleepycat.db.DatabaseException dx)
            {
                dead();
                throw new DatabaseException(_trace.errorPrefix + "Dbc.get: " + dx.getMessage(), dx);
            }

            //
            // For a read-only iterator, we can close the cursor automatically when there
            // are no more entries.
            //
            if(_current == null && _txn == null)
            {
                close();
            }

            if(_current != null)
            {
                _current.iterator(this);
            }

            return _current != null;
        }
        else
        {
            return true;
        }
    }

    @Override
    public java.util.Map.Entry<K, V>
    next()
    {
        if(hasNext())
        {
            _lastReturned = _current;
            return _lastReturned;
        }
        else
        {
            throw new java.util.NoSuchElementException();
        }
    }

    @Override
    public void
    remove()
    {
        if(_txn == null)
        {
            throw new UnsupportedOperationException(
                _trace.errorPrefix + "Cannot remove using an iterator without a transaction");
        }

        //
        // Remove the last object returned by next()
        //
        if(_lastReturned == null)
        {
            throw new IllegalStateException();
        }

        if(_lastReturned == _current)
        {
            try
            {
                if(_cursor.delete() == com.sleepycat.db.OperationStatus.KEYEMPTY)
                {
                    throw new IllegalStateException();
                }
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                dead();
                throw new DeadlockException(_trace.errorPrefix + "Dbc.del: " + e.getMessage(),
                                            _map.connection().currentTransaction(), e);
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                throw new DatabaseException(_trace.errorPrefix + "Dbc.del: " + e.getMessage(), e);
            }
        }
        else
        {
            //
            // Duplicate the cursor and move the _lastReturned element to delete it (using the duplicate).
            //

            //
            // This works only for non-index iterators.
            //
            if(_cursor instanceof com.sleepycat.db.SecondaryCursor)
            {
                throw new UnsupportedOperationException(
                    _trace.errorPrefix + "Cannot remove using an iterator retrieved through an index");
            }

            com.sleepycat.db.Cursor clone = null;

            try
            {
                clone = _cursor.dup(true);

                //
                // Not interested in data.
                //
                com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry();
                dbValue.setPartial(true);

                com.sleepycat.db.OperationStatus rc = clone.getSearchKey(_lastReturned.getDbKey(), dbValue, null);

                if(rc == com.sleepycat.db.OperationStatus.NOTFOUND)
                {
                    throw new IllegalStateException();
                }
                if(clone.delete() == com.sleepycat.db.OperationStatus.KEYEMPTY)
                {
                    throw new IllegalStateException();
                }
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                dead();
                throw new DeadlockException(_trace.errorPrefix + "EntryIterator.remove: " + e.getMessage(),
                                            _map.connection().currentTransaction(), e);
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                throw new DatabaseException(_trace.errorPrefix + "EntryIterator.remove: " + e.getMessage(), e);
            }
            finally
            {
                if(clone != null)
                {
                    closeCursor(clone);
                }
            }
        }
    }

    //
    // Extra operations.
    //
    @Override
    public void
    close()
    {
        if(_iteratorListToken != null)
        {
            _map.removeIterator(_iteratorListToken);
            _iteratorListToken = null;
        }

        if(_cursor != null)
        {
            com.sleepycat.db.Cursor cursor = _cursor;
            _cursor = null;
            closeCursor(cursor);
        }
    }

    //
    // An alias for close()
    //
    @Override
    public void
    destroy()
    {
        close();
    }

    @Override
    protected void
    finalize()
        throws Throwable
    {
        try
        {
            if(_cursor != null)
            {
                _trace.logger.warning(
                    "iterator leaked for Map \"" + _dbName + "\"; the application " +
                    "should have closed it earlier by calling Map.EntryIterator.close(), " +
                    "Map.closeAllIterators(), Map.close(), Connection.close(), or (if also " +
                    "leaking a transaction) Transaction.commit() or Transaction.rollback()");
            }
        }
        catch(java.lang.Exception ex)
        {
        }
        finally
        {
            super.finalize();
        }
    }

    void
    setValue(EntryI<K, V> entry, V value)
    {
        if(_cursor instanceof com.sleepycat.db.SecondaryCursor)
        {
            throw new UnsupportedOperationException(
                _trace.errorPrefix + "Cannot set an iterator retrieved through an index");
        }

        if(_txn == null)
        {
            throw new UnsupportedOperationException(
                _trace.errorPrefix + "Cannot set a value without a transaction");
        }

        //
        // Are we trying to update the current value?
        //
        if(_current == entry)
        {
            //
            // Yes, update it directly
            //
            ByteBuffer v = _map.encodeValue(value);
            com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry(v);

            try
            {
                _cursor.putCurrent(dbValue);
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                dead();
                throw new DeadlockException(_trace.errorPrefix + "Dbc.put: " + e.getMessage(),
                                            _map.connection().currentTransaction(), e);
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                throw new DatabaseException(_trace.errorPrefix + "Dbc.put: " + e.getMessage(), e);
            }
        }
        else
        {
            //
            // Duplicate the cursor and move the entry
            // element to update it (using the duplicate cursor)
            //

            com.sleepycat.db.Cursor clone = null;

            try
            {
                clone = _cursor.dup(true);

                //
                // Not interested in data
                //
                com.sleepycat.db.DatabaseEntry dummy = new com.sleepycat.db.DatabaseEntry();
                dummy.setPartial(true);

                com.sleepycat.db.OperationStatus rc = clone.getSearchKey(entry.getDbKey(), dummy, null);

                if(rc == com.sleepycat.db.OperationStatus.NOTFOUND)
                {
                    NotFoundException ex = new NotFoundException();
                    ex.message = _trace.errorPrefix + "Dbc.get: DB_NOTFOUND";
                    throw ex;
                }

                ByteBuffer v = _map.encodeValue(value);
                com.sleepycat.db.DatabaseEntry dbValue = new com.sleepycat.db.DatabaseEntry(v);
                clone.putCurrent(dbValue);
            }
            catch(com.sleepycat.db.DeadlockException e)
            {
                dead();
                throw new DeadlockException(_trace.errorPrefix + "EntryIterator.setValue: " + e.getMessage(),
                                            _map.connection().currentTransaction(), e);
            }
            catch(com.sleepycat.db.DatabaseException e)
            {
                throw new DatabaseException(_trace.errorPrefix + "EntryIterator.setValue: " + e.getMessage(), e);
            }
            finally
            {
                if(clone != null)
                {
                    closeCursor(clone);
                }
            }
        }
    }

    private void
    closeCursor(com.sleepycat.db.Cursor cursor)
    {
        try
        {
            cursor.close();
        }
        catch(com.sleepycat.db.DeadlockException e)
        {
            dead();
            throw new DeadlockException(_trace.errorPrefix + "Dbc.close: " + e.getMessage(),
                                        _map.connection().currentTransaction(), e);
        }
        catch(com.sleepycat.db.DatabaseException e)
        {
            throw new DatabaseException(_trace.errorPrefix + "Dbc.close: " + e.getMessage(), e);
        }
    }

    private void
    dead()
    {
        if(_cursor != null)
        {
            com.sleepycat.db.Cursor cursor = _cursor;
            _cursor = null;
            closeCursor(cursor);
        }
    }

    private final MapI<K, V> _map;
    private final IteratorModel<K, V> _model;

    private final TraceLevels _trace;
    private final String _dbName; // For use in finalizer.
    private final com.sleepycat.db.Transaction _txn;
    private com.sleepycat.db.Cursor _cursor;
    private EntryI<K, V> _current;
    private EntryI<K, V> _lastReturned;
    private Object _iteratorListToken;
}
