// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef FREEZE_DB_EXCEPTION_ICE
#define FREEZE_DB_EXCEPTION_ICE

module Freeze
{

/**
 *
 * A Freeze database exception.
 *
 * @see DBEnvironment
 * @see DBTransaction
 * @see DB
 * @see Evictor
 *
 **/
exception DBException
{
    /**
     *
     * A message describing the reason for the exception.
     *
     **/
    string message;
};

/**
 *
 * A Freeze database exception, indicating that a database record
 * could not be found.
 *
 **/
exception DBNotFoundException extends DBException
{
};

/**
 *
 * A Freeze database deadlock exception. Transactions can react to
 * this exception by aborting and trying the transaction again.
 *
 **/
exception DBDeadlockException extends DBException
{
};

};

#endif
