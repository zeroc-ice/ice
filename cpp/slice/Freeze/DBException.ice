// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
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
local exception DBException
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
local exception DBNotFoundException extends DBException
{
};

/**
 *
 * A Freeze database deadlock exception. Transactions can react to
 * this exception by aborting and trying the transaction again.
 *
 **/
local exception DBDeadlockException extends DBException
{
};

};

#endif
