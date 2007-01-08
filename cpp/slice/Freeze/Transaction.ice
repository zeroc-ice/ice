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
     **/
    void commit();

    /**
     *
     * Roll back this transaction. 
     *
     **/
    void rollback();
}; 


};

#endif
