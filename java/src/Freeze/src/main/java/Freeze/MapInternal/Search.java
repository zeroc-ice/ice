// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze.MapInternal;

import Freeze.ConnectionI;
import Freeze.DatabaseException;
import Freeze.DeadlockException;
import java.nio.ByteBuffer;

class Search
{
    enum Type
    {
        FIRST       // The first entry
        {
            @Override
            Type descending() { return LAST; };
        },
        LAST        // The last entry
        {
            @Override
            Type descending() { return FIRST; };
        },
        CEILING     // The entry with the smallest key greater than or equal to the target key
        {
            @Override
            Type descending() { return FLOOR; };
        },
        FLOOR       // The entry with the greatest key less than or equal to the target key
        {
            @Override
            Type descending() { return CEILING; };
        },
        HIGHER      // The entry with the smallest key greater than the target key
        {
            @Override
            Type descending() { return LOWER; };
        },
        LOWER       // The entry with the greatest key less than the target key
        {
            @Override
            Type descending() { return HIGHER; };
        };

        abstract Type descending(); // Returns the descending (opposite) value.
    }

    interface KeyValidator
    {
        boolean keyInRange(ByteBuffer key);
    }

    static boolean
    search(Type type, ConnectionI connection, String dbName, com.sleepycat.db.Database db,
           com.sleepycat.db.DatabaseEntry key, com.sleepycat.db.DatabaseEntry value,
           java.util.Comparator<ByteBuffer> comparator, KeyValidator validator, TraceLevels trace)
    {
        if(type != Type.FIRST && type != Type.LAST && comparator == null)
        {
            throw new UnsupportedOperationException("A comparator is required");
        }

        if(trace.level >= 2)
        {
            trace.logger.trace("Freeze.Map", "searching Db \"" + dbName + "\"");
        }

        if(value == null)
        {
            value = new com.sleepycat.db.DatabaseEntry();
            value.setPartial(true); // Not interested in the value.
        }

        try
        {
            for(;;)
            {
                com.sleepycat.db.Cursor dbc = null;
                try
                {
                    com.sleepycat.db.DatabaseEntry dbcKey =
                        new com.sleepycat.db.DatabaseEntry(key != null ? UtilI.getBuffer(key) : null);
                    dbcKey.setReuseBuffer(false);

                    dbc = db.openCursor(connection.dbTxn(), null);

                    com.sleepycat.db.OperationStatus status = null;

                    switch(type)
                    {
                    case FIRST:
                    {
                        status = dbc.getFirst(dbcKey, value, null);
                        break;
                    }
                    case LAST:
                    {
                        status = dbc.getLast(dbcKey, value, null);
                        break;
                    }
                    case CEILING:
                    {
                        //
                        // The semantics of getSearchKeyRange match CEILING.
                        //
                        status = dbc.getSearchKeyRange(dbcKey, value, null);
                        break;
                    }
                    case FLOOR:
                    {
                        status = dbc.getSearchKeyRange(dbcKey, value, null);
                        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                        {
                            //
                            // getSearchKeyRange returns the smallest key greater than or equal to
                            // the target key. If the matching key is greater than the target key
                            // then we need to get the previous entry.
                            //
                            int cmp = comparator.compare(UtilI.getBuffer(dbcKey), UtilI.getBuffer(key));
                            assert(cmp >= 0);
                            if(cmp > 0)
                            {
                                status = dbc.getPrevNoDup(dbcKey, value, null);
                            }
                        }
                        else if(status == com.sleepycat.db.OperationStatus.NOTFOUND)
                        {
                            //
                            // All keys must be less than the target key so we pick the largest of
                            // all (the last one).
                            //
                            status = dbc.getLast(dbcKey, value, null);
                        }
                        break;
                    }
                    case HIGHER:
                    {
                        status = dbc.getSearchKeyRange(dbcKey, value, null);
                        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                        {
                            //
                            // getSearchKeyRange returns the smallest key greater than or equal to
                            // the target key. If the matching key is equal to the target key
                            // then we need to get the next entry.
                            //
                            int cmp = comparator.compare(UtilI.getBuffer(dbcKey), UtilI.getBuffer(key));
                            assert(cmp >= 0);
                            if(cmp == 0)
                            {
                                status = dbc.getNextNoDup(dbcKey, value, null);
                            }
                        }
                        break;
                    }
                    case LOWER:
                    {
                        status = dbc.getSearchKeyRange(dbcKey, value, null);
                        if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                        {
                            //
                            // getSearchKeyRange returns the smallest key greater than or equal to
                            // the target key. We move to the previous entry, whose key must be less
                            // than the target key.
                            //
                            status = dbc.getPrevNoDup(dbcKey, value, null);
                        }
                        else if(status == com.sleepycat.db.OperationStatus.NOTFOUND)
                        {
                            //
                            // All keys must be less than the target key so we pick the largest of
                            // all (the last one).
                            //
                            status = dbc.getLast(dbcKey, value, null);
                        }
                        break;
                    }
                    }

                    if(status == com.sleepycat.db.OperationStatus.SUCCESS)
                    {
                        if(validator == null || validator.keyInRange(UtilI.getBuffer(dbcKey)))
                        {
                            key.setDataNIO(UtilI.getBuffer(dbcKey));
                            return true;
                        }
                    }

                    return false;
                }
                catch(com.sleepycat.db.DeadlockException dx)
                {
                    if(connection.dbTxn() != null)
                    {
                        throw new DeadlockException(trace.errorPrefix + dx.getMessage(),
                                                    connection.currentTransaction(), dx);
                    }
                    else
                    {
                        if(trace.deadlockWarning)
                        {
                            trace.logger.warning("Deadlock in Freeze.Map while searching \"" + dbName +
                                                 "\"; retrying...");
                        }

                        //
                        // Retry
                        //
                    }
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
                            //
                            // Ignored
                            //
                        }
                    }
                }
            }
        }
        catch(com.sleepycat.db.DatabaseException dx)
        {
            throw new DatabaseException(trace.errorPrefix + dx.getMessage(), dx);
        }
    }
}
