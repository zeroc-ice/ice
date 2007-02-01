// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef UTIL_H
#define UTIL_H

#include <occi.h>
#include <string>

//
// Grabs a connection from a connection pool and ensures
// it's properly released when the ConnectionHolder is destroyed 
//
class ConnectionHolder : public IceUtil::Shared
{
public:

    ConnectionHolder(oracle::occi::StatelessConnectionPool*);
  
    oracle::occi::Connection* connection() const
    {
        return _con;
    }

    void commit();
    void rollback();

protected:
    virtual ~ConnectionHolder();

private:

    oracle::occi::Connection* _con;
    bool _txDone;
    oracle::occi::StatelessConnectionPool* _pool;
};

typedef IceUtil::Handle<ConnectionHolder> ConnectionHolderPtr;

//
// Create a fresh exception-safe statement (typically on the stack)
//
class StatementHolder
{
public:
    StatementHolder(oracle::occi::Connection*);
    StatementHolder(const ConnectionHolderPtr&);
    ~StatementHolder();

    oracle::occi::Statement* statement() const
    {
        return _stmt;
    }
    
private:
    oracle::occi::Connection* _con;
    oracle::occi::Statement* _stmt;
};


//
// Encode/decode a RefAny into a string
//
std::string encodeRef(const oracle::occi::RefAny&, oracle::occi::Environment*);
oracle::occi::RefAny decodeRef(const std::string&, oracle::occi::Environment*v, oracle::occi::Connection*);

#endif
