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
