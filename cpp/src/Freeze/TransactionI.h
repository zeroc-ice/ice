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

#ifndef FREEZE_TRANSACTIONI_H
#define FREEZE_TRANSACTIONI_H

#include <Freeze/Transaction.h>
#include <db_cxx.h>

namespace Freeze
{

class ConnectionI;
typedef IceUtil::Handle<ConnectionI> ConnectionIPtr;

class TransactionI : public Transaction
{
public:

    virtual void
    commit();

    virtual void
    rollback();
    
    TransactionI(ConnectionI*);
    
    ~TransactionI();
    
    DbTxn*
    dbTxn() const
    {
	return _txn;
    }

private:
    
    void
    cleanup();

    ConnectionIPtr _connection;
    DbTxn* _txn;
};

typedef IceUtil::Handle<TransactionI> TransactionIPtr;

}
#endif
