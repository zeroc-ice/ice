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

/**
 *
 * The Ice module for object persistence.
 *
 **/
module Freeze
{

local interface DB;
local interface DBEnv;

/**
 *
 * A database exception.
 *
 **/
struct DBException
{
    string message;
};

/**
 *
 * An object representing a database.
 *
 * @see DBException
 * @see DBEnv
 *
 **/
local interface DB
{
    /**
     *
     * Put a Servant into the database, using a string as
     * key. Typically, applications will use the identity of the Ice
     * Object implemented by the Servant as key.
     *
     * @param key The key under which the servant will be stored in
     * the database.
     *
     * @param servant The servant to store. If the servant is null,
     * this operation does nothing.
     *
     * @see get
     * @see del
     *
     **/
    void put(string key, Object servant);

    /**
     *
     * Get a Servant from the database by it's key.
     *
     * @param key The key under which the servant is stored in the database.
     *
     * @return The Servant from the database, or null if the key does
     * not exist.
     *
     * @see put
     * @see del
     *
     **/
    Object get(string key);

    /**
     *
     * Remove a key and the corresponding Servant from the
     * database. If the key does not exist, this operation will do
     * nothing.
     *
     * @param key The key to remove.
     *
     * @see put
     * @see get
     *
     **/
    void del(string key);

    /**
     *
     * Close the database and destroy this database object. Subsequent
     * calls to <literal>close</literal> have no effect.
     *
     * @see DBEnv::open
     * @see DBEnv::close
     *
     **/
    void close();
};

/**
 *
 * An object representing a database environment.
 *
 * @see DBException
 * @see DB
 *
 **/
local interface DBEnv
{
    /**
     *
     * Open a database and return a database object for this
     * database. If the database has been opened before, the
     * previously returned database object is returned again.
     *
     * @param name The database name.
     *
     * @return The database object.
     *
     * @see DB
     * @see DB::close
     *
     **/
    DB open(string name) throws DBException;

    /**
     *
     * Close the database environment and destroy this database
     * environment object. This operation also calls
     * <literal>close</literal> on all databases that have been opened
     * with this database environment object. Subsequent
     * calls to <literal>close</literal> have no effect.
     *
     * @see DB::close
     *
     **/
    void close() throws DBException;
};

};

#endif
