// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <HR.h>
#include <Util.h>
#include <sstream>

using namespace std;
using namespace oracle::occi;

ConnectionHolder::ConnectionHolder(StatelessConnectionPool* pool) 
    : _con(pool->getAnyTaggedConnection()),
      _pool(pool)
{
}

ConnectionHolder::~ConnectionHolder()
{
    if(_con != 0)
    {
	try
	{
	    rollback();
	}
	catch(...)
	{
	    // ignored, to avoid a crash during 
	    // stack unwinding
	}
    }
}

void
ConnectionHolder::commit()
{
    assert(_con != 0);
    try
    {
	_con->commit();
    }
    catch(const SQLException& e)
    {
	release();
	throw HR::SqlException(e.what());
    }
    catch(...)
    {
	release();
	throw;
    }
    release();
}

void
ConnectionHolder::rollback()
{
    assert(_con != 0);
    try
    {
	_con->rollback();
    }
    catch(const SQLException& e)
    {
	release();
	throw HR::SqlException(e.what());
    }
    catch(...)
    {
	release();
	throw;
    }
    release();
}

void
ConnectionHolder::release()
{
    Connection* con = _con;
    _con = 0;
    _pool->releaseConnection(con);
}

StatementHolder::StatementHolder(Connection* con) :
    _stmt(con->createStatement()),
    _con(con)
{
}

StatementHolder::StatementHolder(ConnectionHolder& conh) :
    _stmt(conh.connection()->createStatement()),
    _con(conh.connection())
{
}

StatementHolder::~StatementHolder()
{
    _con->terminateStatement(_stmt);
}

int decodeName(const string& name)
{
    int result = 0;
    istringstream is(name);
    is >> result;
    if(!is || !is.eof())
    {
	cerr << "Unable to decode " << name << endl;
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
    return result;
}

string encodeName(int n)
{
    ostringstream os;
    os << n;
    return os.str();
}

