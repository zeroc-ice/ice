// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_DB_ICE
#define FREEZE_DB_ICE

#include <Freeze/DBF.ice>
#include <Freeze/EvictorF.ice>

/**
 *
 * The Ice module for object persistence.
 *
 **/
module Freeze
{

/**
 *
 * A Freeze database exception.
 *
 **/
struct DBException
{
    /**
     *
     * A message describing the exception.
     *
     **/
    string message;
};

/**
 *
 * A database environment. Multiple databases can be created from a
 * single database environment. The database environment also offers
 * operations to create transactions.
 *
 * @see DBException
 * @see DBTransaction
 * @see DB
 * @see DBForServants
 *
 **/
local interface DBEnvironment
{
    /**
     *
     * Get the name of the database environment.
     *
     * @return The name of this database environment.
     *
     **/
    string getName();
     
    /**
     *
     * Open and return a basic key/value database object. If the
     * database has been opened before, the previously returned
     * database object is returned again.
     *
     * @param name The database name.
     *
     * @return The database object.
     *
     * @see DB
     * @see DB::close
     *
     **/
    DB openDB(string name) throws DBException;

    /**
     *
     * Open and return a databae for identity/Servant pairs. If the
     * database has been opened before, the previously returned
     * database object is returned again.
     *
     * @param name The database name.
     *
     * @return The database object.
     *
     * @see DBForServant
     * @see DBForServant::close
     *
     **/
    DB openDBForServant(string name) throws DBException;

    /**
     *
     * Start a new transaction in this database environment, and
     * return the transaction object for such new transaction.
     *
     * @return The transaction object.
     *
     * @see DBTransaction
     *
     **/
    DBTransaction startDBTransaction() throws DBException;

    /**
     *
     * Close the database environment and destroy this database
     * environment object. This operation also calls
     * <literal>close</literal> on all databases that have been opened
     * with this database environment object. Subsequent calls to
     * <literal>close</literal> have no effect.
     *
     * @see DB::close
     *
     **/
    void close() throws DBException;
};

/**
 *
 * A transaction object.
 *
 **/
local interface DBTransaction
{
    /**
     *
     * Commit a transaction.
     *
     **/
    void commit() throws DBException;

    /**
     *
     * Abort a transaction.
     *
     **/
    void abort() throws DBException;
};

/**
 *
 * A database key, represented as a sequence of bytes.
 *
 **/
sequence<byte> Key;

/**
 *
 * A database value, represented as a sequence of bytes
 *
 **/
sequence<byte> Value;

/**
 *
 * A basic database, which can store key/value pairs.
 *
 * @see DBException
 * @see DBEnvironment
 *
 **/
local interface DB
{
    /**
     *
     * Get the name of the database.
     *
     * @return The name of this database.
     *
     **/
    string getName();

    /**
     *
     * Save a value in the database under a given key.
     *
     * @param key The key under which the value will be stored in
     * the database.
     *
     * @param servant The value to store.
     *
     * @param txn If true, do a transaction-protected write to the
     * database.
     *
     * @see get
     * @see del
     * @see DBTransaction
     *
     **/
    void put(Key key, Value value, bool txn) throws DBException;

    /**
     *
     * Get a value from a database by it's key.
     *
     * @param key The key under which the value is stored in the database.
     *
     * @return The value from the database, or an empty value if the
     * key does not exist.
     *
     * @see put
     * @see del
     *
     **/
    Value get(Key key) throws DBException;

    /**
     *
     * Remove a key and the corresponding value from the database. If
     * the key does not exist, this operation will do nothing.
     *
     * @param key The key to remove together with the corresponding
     * value.
     *
     * @see put
     * @see get
     *
     **/
    void del(Key key) throws DBException;

    /**
     *
     * Close the database and destroy this database object. Subsequent
     * calls to <literal>close</literal> have no effect.
     *
     * @see DBEnvironment::openDB
     * @see DBEnvironment::close
     *
     **/
    void close() throws DBException;
};

/**
 *
 * A Servant database, which can store identity/Servant pairs.
 *
 * @see DBException
 * @see DBEnvironment
 * @see Evictor
 *
 **/
local interface DBForServants
{
    /**
     *
     * Get the name of the database.
     *
     * @return The name of this database.
     *
     **/
    string getName();

    /**
     *
     * Put a Servant into the database, using the Ice Object's
     * identity implemented by the Servant as a key.
     *
     * @param identity The identity under which the servant will be
     * stored in the database.
     *
     * @param servant The servant to store. If the servant is null,
     * this operation does nothing.
     *
     * @param txn If true, do a transaction-protected write to the
     * database.
     *
     * @see getServant
     * @see delServant
     * @see DBTransaction
     *
     **/
    void putServant(string identity, Object servant, bool txn) throws DBException;

    /**
     *
     * Get a Servant from the database by the Ice Object's identity
     * which the Servant must implement.
     *
     * @param identity The identity under which the servant is stored
     * in the database.
     *
     * @return The Servant from the database, or null if the identity
     * does not exist.
     *
     * @see putServant
     * @see delServant
     *
     **/
    Object getServant(string identity) throws DBException;

    /**
     *
     * Remove an identity and the corresponding Servant from the
     * database. If the identity does not exist, this operation will
     * do nothing.
     *
     * @param identity The identity to remove together with the
     * corresponding Servant.
     *
     * @see putServant
     * @see getServant
     *
     **/
    void delServant(string identity) throws DBException;

    /**
     *
     * Create a new Evictor that uses this database.
     *
     * @return The new Evictor.
     *
     * @see Evictor
     *
     **/
    Evictor createEvictor();

    /**
     *
     * Close the database and destroy this database object. Subsequent
     * calls to <literal>close</literal> have no effect.
     *
     * @see DBEnvironment::openDBForServants
     * @see DBEnvironment::close
     *
     **/
    void close() throws DBException;
};

};

#endif
