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

#include <Freeze/TransactionHolder.h>

using namespace Freeze;

Freeze::TransactionHolder::TransactionHolder(const ConnectionPtr& connection) 
    : _transaction(0)
{
    if(connection->currentTransaction() == 0)
    {
	_transaction = connection->beginTransaction();
    }
}

Freeze::TransactionHolder::~TransactionHolder()
{
    try
    {
	rollback();
    }
    catch(...)
    {
	//
	// Ignored to avoid crash during stack unwinding
	//
    }
}

void
Freeze::TransactionHolder::commit()
{
    if(_transaction != 0)
    {
	try
	{
	    _transaction->commit();
	    _transaction = 0;
	}
	catch(...)
	{
	    _transaction = 0;
	    throw;
	}
    }
}

void
Freeze::TransactionHolder::rollback()
{
    if(_transaction != 0)
    {
	try
	{
	    _transaction->rollback();
	    _transaction = 0;
	}
	catch(...)
	{
	    _transaction = 0;
	    throw;
	}
    }
}
