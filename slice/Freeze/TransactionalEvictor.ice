// **********************************************************************
//
// Copyright (c) 2003-2015 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

[["cpp:header-ext:h", "objc:header-dir:objc"]]

#include <Freeze/Evictor.ice>

module Freeze
{

local interface Transaction;

/**
 * 
 * A transactional evictor is an evictor that performs every single read-write
 * operation within its own transaction.
 *
 **/
local interface TransactionalEvictor extends Evictor
{
    /**
     *
     * Get the transaction associated with the calling thread.
     *
     * @return The transaction associated with the calling thread.
     *
     **/
    ["cpp:const"] Transaction getCurrentTransaction();

    /**
     *
     * Associate a transaction with the calling thread.
     *
     * @param tx The transaction to associate with the calling thread.
     *
     **/
    void setCurrentTransaction(Transaction tx);
};

};


