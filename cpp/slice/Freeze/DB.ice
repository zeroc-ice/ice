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
local class DBFactory;

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
 * @see DBFactory
 *
 **/
local class DB
{
    /**
     *
     * Open the database.
     *
     **/
    void open(string name) throws DBException;

    /**
     *
     * Close the database.
     *
     **/
    void close() throws DBException;

    /**
     *
     * Destroy this database object. If the database has not been
     * closed before, <literal>destroy</literal> also closes the
     * database.
     *
     **/
    void destroy();
};

/**
 *
 * A factory for database objects.
 *
 * @see DBException
 * @see DB
 *
 **/
local class DBFactory
{
    /**
     *
     * Create a new database object.
     *
     * @return The new database object.
     *
     * @see DB
     *
     **/
    DB createDB() throws DBException;

    /**
     *
     * Destroy this factory and all database objects that have been created by this factory.
     *
     **/
    void destroy() throws DBException;
};

};

#endif
