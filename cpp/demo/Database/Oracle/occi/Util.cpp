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

string
encodeRef(const RefAny& ref, Environment* env)
{
    string result;

    ub4 length = OCIRefHexSize(env->getOCIEnvironment(), ref.getRef());
    OraText* buffer = new OraText[length];

    OCIError* error = 0;
    OCIHandleAlloc(env->getOCIEnvironment(), reinterpret_cast<void**>(&error), OCI_HTYPE_ERROR, 0, 0);
    sword status = OCIRefToHex(env->getOCIEnvironment(), error, ref.getRef(), buffer, &length);

    if(status == OCI_SUCCESS)
    {
	result.assign(reinterpret_cast<char*>(buffer), length);
    }
    else
    {
	cerr << "encodeRef failed: ";
	sb4 errcode = 0;
	OraText buf[512];
	OCIErrorGet(error, 1, 0, &errcode, buf, 512, OCI_HTYPE_ERROR);
	cerr << reinterpret_cast<char*>(buf) << endl;
    }

    OCIHandleFree(error, OCI_HTYPE_ERROR);
    delete[] buffer;
    return result;
}

RefAny
decodeRef(const string& str, Environment* env, Connection* con)
{
    OCIRef* ref = 0;
    OCIError* error = 0;
    OCIHandleAlloc(env->getOCIEnvironment(), reinterpret_cast<void**>(&error), OCI_HTYPE_ERROR, 0, 0);

    sword status = OCIRefFromHex(env->getOCIEnvironment(), error,
				 con->getOCIServiceContext(), 
				 reinterpret_cast<const OraText*>(str.c_str()), str.length(),
				 &ref);

    if(status == OCI_SUCCESS)
    {	
	OCIHandleFree(error, OCI_HTYPE_ERROR);
	return RefAny(con, ref);
    }
    else
    {
	cerr << "decodeRef failed: ";
	sb4 errcode = 0;
	OraText buf[512];
	OCIErrorGet(error, 1, 0, &errcode, buf, 512, OCI_HTYPE_ERROR);
	cerr << reinterpret_cast<char*>(buf) << endl;

	OCIHandleFree(error, OCI_HTYPE_ERROR);
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
}

