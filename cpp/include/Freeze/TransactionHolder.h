// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************


#ifndef FREEZE_TRANSACTION_HOLDER_H
#define FREEZE_TRANSACTION_HOLDER_H

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

#endif
