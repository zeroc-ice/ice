// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

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
["cpp:ice_print"]
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
["cpp:ice_print"]
local exception NotFoundException extends DatabaseException
{
};

/**
 *
 * A Freeze database deadlock exception. Applications can react to
 * this exception by aborting and trying the transaction again.
 *
 **/
["cpp:ice_print"]
local exception DeadlockException extends DatabaseException
{
    /**
     *
     * The transaction in which the deadlock occurred.
     *
     **/
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
 * Exception raised when Freeze fails to locate an index.
 *
 **/
["cpp:ice_print"]
local exception IndexNotFoundException
{
    /**
     *
     * The name of the map in which the index could not be found.
     *
     **/
    string mapName;

    /**
     *
     * The name of the index.
     *
     **/
    string indexName;
};

};

