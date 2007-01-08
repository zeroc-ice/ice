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
// it's properly released when ConnectionHolder goes out
// scope
//
class ConnectionHolder
{
public:

    ConnectionHolder(oracle::occi::StatelessConnectionPool*);
    ~ConnectionHolder();

    oracle::occi::Connection* connection() const
    {
	return _con;
    }

    void commit();
    void rollback();

private:

    void release();

    oracle::occi::Connection* _con;
    oracle::occi::StatelessConnectionPool* _pool;
};


//
// Create a fresh exception-safe statement
//
class StatementHolder
{
public:
    StatementHolder(oracle::occi::Connection*);
    StatementHolder(ConnectionHolder&);
    ~StatementHolder();

    oracle::occi::Statement* statement() const
    {
	return _stmt;
    }
    
private:
    oracle::occi::Connection* _con;
    oracle::occi::Statement* _stmt;
};


int decodeName(const std::string&);
std::string encodeName(int);

#endif
