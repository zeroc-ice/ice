// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package Freeze;

public class Util
{
    /**
     * Returns a new background save evictor. The Berkeley DB database with the
     * name given in <code>filename</code> is opened if it already exists.
     * If the database does not exist yet and <code>createDb</code>
     * is true, the database is created automatically; otherwise,
     * <code>DatabaseException</code> is raised.
     *
     * @param adapter The object adapter to associate with this evictor.
     * @param envName The name of the Berkeley DB database environment. The
     *   environment must already exist.
     * @param filename The name of the Berkeley DB database file.
     * @param initializer An object that is used to initialize a servant after its
     *   state is restored from the database, or <code>null</code> if no initializer
     *   is necessary.
     * @param indices An array of Freeze indices, or <code>null</code> if no indices
     *   are defined.
     * @param createDb True if the Berkeley DB database should be created if it does
     *   not exist, false otherwise.
     * @return A new background save evictor.
     * @throws Freeze.DatabaseException If an error occurs during database operations.
     */
    public static BackgroundSaveEvictor
    createBackgroundSaveEvictor(Ice.ObjectAdapter adapter, String envName, String filename,
                                ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        return new BackgroundSaveEvictorI(adapter, envName, filename, initializer, indices, createDb);
    }

    /**
     * Returns a new background save evictor. The Berkeley DB database with the
     * name given in <code>filename</code> is opened if it already exists.
     * If the database does not exist yet and <code>createDb</code>
     * is true, the database is created automatically; otherwise,
     * <code>DatabaseException</code> is raised.
     *
     * @param adapter The object adapter to associate with this evictor.
     * @param envName The name of the Berkeley DB database environment. The
     *   environment must already exist.
     * @param dbEnv The database environment in which to open the database.
     * @param filename The name of the Berkeley DB database file.
     * @param initializer An object that is used to initialize a servant after its
     *   state is restored from the database, or <code>null</code> if no initializer
     *   is necessary.
     * @param indices An array of Freeze indices, or <code>null</code> if no indices
     *   are defined.
     * @param createDb True if the Berkeley DB database should be created if it does
     *   not exist, false otherwise.
     * @return A new background save evictor.
     * @throws Freeze.DatabaseException If an error occurs during database operations.
     */
    public static BackgroundSaveEvictor
    createBackgroundSaveEvictor(Ice.ObjectAdapter adapter, String envName, com.sleepycat.db.Environment dbEnv,
                                String filename, ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        return new BackgroundSaveEvictorI(adapter, envName, dbEnv, filename, initializer, indices, createDb);
    }

    /**
     * Returns a new transactional evictor. The Berkeley DB database with the
     * name given in <code>filename</code> is opened if it already exists.
     * If the database does not exist yet and <code>createDb</code>
     * is true, the database is created automatically; otherwise,
     * <code>DatabaseException</code> is raised.
     *
     * @param adapter The object adapter to associate with this evictor.
     * @param envName The name of the Berkeley DB database environment. The
     *   environment must already exist.
     * @param filename The name of the Berkeley DB database file.
     * @param facetTypes A map of facet name to Slice type id, or <code>null</code> if
     *   no map is necessary.
     * @param initializer An object that is used to initialize a servant after its
     *   state is restored from the database, or <code>null</code> if no initializer
     *   is necessary.
     * @param indices An array of Freeze indices, or <code>null</code> if no indices
     *   are defined.
     * @param createDb True if the Berkeley DB database should be created if it does
     *   not exist, false otherwise.
     * @return A new transactional evictor.
     * @throws Freeze.DatabaseException If an error occurs during database operations.
     */
    public static TransactionalEvictor
    createTransactionalEvictor(Ice.ObjectAdapter adapter, String envName, String filename,
                               java.util.Map<String, String> facetTypes, ServantInitializer initializer,
                               Index[] indices, boolean createDb)
    {
        return new TransactionalEvictorI(adapter, envName, filename, facetTypes, initializer, indices, createDb);
    }

    /**
     * Returns a new transactional evictor. The Berkeley DB database with the
     * name given in <code>filename</code> is opened if it already exists.
     * If the database does not exist yet and <code>createDb</code>
     * is true, the database is created automatically; otherwise,
     * <code>DatabaseException</code> is raised.
     *
     * @param adapter The object adapter to associate with this evictor.
     * @param envName The name of the Berkeley DB database environment. The
     *   environment must already exist.
     * @param dbEnv The database environment in which to open the database.
     * @param filename The name of the Berkeley DB database file.
     * @param facetTypes A map of facet name to Slice type id, or <code>null</code> if
     *   no map is necessary.
     * @param initializer An object that is used to initialize a servant after its
     *   state is restored from the database, or <code>null</code> if no initializer
     *   is necessary.
     * @param indices An array of Freeze indices, or <code>null</code> if no indices
     *   are defined.
     * @param createDb True if the Berkeley DB database should be created if it does
     *   not exist, false otherwise.
     * @return A new transactional evictor.
     * @throws Freeze.DatabaseException If an error occurs during database operations.
     */
    public static TransactionalEvictor
    createTransactionalEvictor(Ice.ObjectAdapter adapter, String envName, com.sleepycat.db.Environment dbEnv,
                               String filename, java.util.Map<String, String> facetTypes,
                               ServantInitializer initializer, Index[] indices, boolean createDb)
    {
        return new TransactionalEvictorI(adapter, envName, dbEnv, filename, facetTypes, initializer, indices, createDb);
    }

    /**
     * Creates a new Freeze connection using the given communicator and database
     * environment.
     *
     * @param communicator The communicator associated with this connection.
     * @param envName The name of the Berkeley DB database environment.
     * @return A new Freeze connection.
     * @throws Freeze.DatabaseException If an error occurs during database operations.
     */
    public static Connection
    createConnection(Ice.Communicator communicator, String envName)
    {
        return new ConnectionI(communicator, envName, null);
    }

    /**
     * Creates a new Freeze connection using the given communicator and database
     * environment.
     *
     * @param communicator The communicator associated with this connection.
     * @param envName The name of the Berkeley DB database environment.
     * @param dbEnv The database environment associated with this connection.
     * @return A new Freeze connection.
     * @throws Freeze.DatabaseException If an error occurs during database operations.
     */
    public static Connection
    createConnection(Ice.Communicator communicator, String envName, com.sleepycat.db.Environment dbEnv)
    {
        return new ConnectionI(communicator, envName, dbEnv);
    }

    /**
     * Returns the database file name of the Freeze catalog.
     *
     * @return The database file name.
     */
    public static String
    catalogName()
    {
        return _catalogName;
    }

    /**
     * Returns the database file name of the Freeze catalog index list.
     *
     * @return The database file name.
     */
    public static String
    catalogIndexListName()
    {
        return _catalogIndexListName;
    }

    /**
     * Returns the Berkeley DB transaction object associated with a Freeze transaction.
     *
     * @param tx The Freeze transaction.
     * @return The Berkeley DB transaction.
     */
    public static com.sleepycat.db.Transaction
    getTxn(Transaction tx)
    {
        try
        {
            return ((TransactionI)tx).dbTxn();
        }
        catch(ClassCastException e)
        {
            return null;
        }
    }

    /**
     * Registers a handler for fatal errors encountered by the background save evictor.
     *
     * @param cb The error callback.
     */
    public static synchronized FatalErrorCallback
    registerFatalErrorCallback(FatalErrorCallback cb)
    {
        FatalErrorCallback result = _fatalErrorCallback;
        _fatalErrorCallback = cb;
        return result;
    }

    static synchronized void
    handleFatalError(BackgroundSaveEvictor evictor, Ice.Communicator communicator, RuntimeException ex)
    {
        if(_fatalErrorCallback != null)
        {
            _fatalErrorCallback.handleError(evictor, communicator, ex);
        }
        else
        {
            communicator.getLogger().error("*** Halting JVM ***");
            Runtime.getRuntime().halt(1);
        }
    }

    private static String _catalogName = "__catalog";
    private static String _catalogIndexListName = "__catalogIndexList";
    private static FatalErrorCallback _fatalErrorCallback = null;
}
