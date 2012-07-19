// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <Freeze/Connection.h>
#include <Freeze/Transaction.h>

namespace Freeze
{

class FREEZE_API TransactionHolder
{
public:
    
    TransactionHolder(const ConnectionPtr&);
    
    ~TransactionHolder();

    void
    commit();

    void
    rollback();

private:
    
    //
    // Not implemented
    //
    TransactionHolder(const TransactionHolder&);

    TransactionHolder& 
    operator=(const TransactionHolder&);

    TransactionPtr _transaction;
};

}
