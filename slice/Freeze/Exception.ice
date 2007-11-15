// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
