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
