// **********************************************************************
//
// Copyright (c) 2003-2010 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_EXCEPTION_ICE
#define FREEZE_EXCEPTION_ICE

[["cpp:header-ext:h"]]

module Freeze
{

local interface Transaction;

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
    Transaction tx;
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

/**
 *
 * Exception raised when Freeze fails to locate an index
 *
 **/
local exception IndexNotFoundException
{
    string mapName;
    string indexName;
};

};

#endif
