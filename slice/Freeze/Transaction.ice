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

local interface Connection;


/**
 * 
 * A transaction. If you want to use a transaction concurrently 
 * in multiple threads, you need to serialize access to this
 * transaction.
 *
 **/
local interface Transaction
{
    /**
     *
     * Commit this transaction. 
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     **/
    void commit();

    /**
     *
     * Roll back this transaction. 
     *
     * @throws DatabaseException Raised if a database failure occurred.
     *
     **/
    void rollback();

    /**
     *
     * Get the connection associated with this Transaction
     *
     **/
    ["cpp:const"] Connection getConnection();
}; 


};

