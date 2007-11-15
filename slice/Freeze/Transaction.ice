// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_TRANSACTION_ICE
#define FREEZE_TRANSACTION_ICE

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

#endif
