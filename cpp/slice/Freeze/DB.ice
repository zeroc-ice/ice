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

local class DB;
local class DBEnv;

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
local class DB
{
    /**
     *
     * Close the database and destroy this database object. Subsequent
     * calls to <literal>close</literal> have no effect.
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
local class DBEnv
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
     **/
    void close() throws DBException;
};

};

#endif
