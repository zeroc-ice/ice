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
 * The base class for all Freeze exceptions.
 *
 **/
local class Exception
{
    /**
     *
     * Get a message describing the failure that caused the exception.
     *
     * @return The message describing the exception.
     *
     **/
    string message();
};

/**
 *
 * This exception is raised by <literal>DB</literal>
 * operations.
 *
 * @see DB
 *
 **/
local class DBException extends Exception
{
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
 * This exception is raised by <literal>DBFactory</literal>
 * operations.
 *
 * @see DBFactory
 *
 **/
local class DBFactoryException extends Exception
{
};

/**
 *
 * A factory for database objects.
 *
 * @see DBFactoryException
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
    DB createDB() throws DBFactoryException;

    /**
     *
     * Destroy this factory and all database objects that have been created by this factory.
     *
     **/
    void destroy() throws DBFactoryException, DBException;
};

};

#endif
