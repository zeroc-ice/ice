// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef FREEZE_EXCEPTION_ICE
#define FREEZE_EXCEPTION_ICE

module Freeze
{

/**
 *
 * A Freeze database exception.
 *
 * @see DB
 * @see Evictor
 * @see Connection
 *
 **/
local exception DatabaseException
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
local exception NotFoundException extends DatabaseException
{
};

/**
 *
 * A Freeze database deadlock exception. Applications can react to
 * this exception by aborting and trying the transaction again.
 *
 **/
local exception DeadlockException extends DatabaseException
{
};

/**
 *
 * This Freeze Iterator is not on a valid position, for example
 * this position has been erased.
 *
 **/
local exception InvalidPositionException
{
};


};

#endif
