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

#ifndef FREEZE_CONNECTION_ICE
#define FREEZE_CONNECTION_ICE

#include <Freeze/Transaction.ice>
#include <Ice/CommunicatorF.ice>

module Freeze
{

local exception TransactionAlreadyInProgressException
{
};


/**
 * 
 * A connection to a database (database environment with Berkeley
 * DB). If you want to use a connection concurrently 
 * in multiple threads, you need to serialize access to this
 * connection.
 *
 **/
local interface Connection
{
    /**
     *
     * Create a new transaction. Only one transaction at a time can
     * be associated with a connection. 
     *
     * @return The new transaction
     *
     * @throws raises TransactionAlreadyInProgressException if a 
     * transaction is already associated with this connection.
     *
     **/
    Transaction beginTransaction();
    
    
    /**
     *
     * Returns the transaction associated with this connection. 
     *
     * @return The current transaction if there is one, null otherwise.
     *
     **/
    nonmutating Transaction currentTransaction();

     
    /**
     *
     * Closes this connection. If there is an associated transaction,
     * it is rolled back.
     *
     **/
    void close();

    
    /**
     *
     * Returns the communicator associated with this connection
     *
     **/
    nonmutating Ice::Communicator getCommunicator();

    /**
     *
     * The name of the connected system (e.g. Berkeley DB
     * environment)
     *
     **/
    nonmutating string getName();

}; 

};

#endif
