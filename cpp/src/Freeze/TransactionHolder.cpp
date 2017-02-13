// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
