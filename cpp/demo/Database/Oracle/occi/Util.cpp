// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

ConnectionHolder::ConnectionHolder(StatelessConnectionPool* pool) : 
    _con(pool->getAnyTaggedConnection()),
    _txDone(false),
    _pool(pool)
{
}

void
ConnectionHolder::commit()
{
    _txDone = true;
    try
    {
        _con->commit();
    }
    catch(const SQLException& e)
    {
        throw HR::SqlException(e.what());
    }
}

void
ConnectionHolder::rollback()
{
    _txDone = true;
    try
    {
        _con->rollback();
    }
    catch(const SQLException& e)
    {
        throw HR::SqlException(e.what());
    }
}

ConnectionHolder::~ConnectionHolder()
{
    if(!_txDone)
    {
        _txDone = true;
        try
        {
            _con->rollback();
        }
        catch(const std::exception&)
        {
        }
    }

    try
    {
        _pool->releaseConnection(_con);
    }
    catch(const std::exception&)
    {
    }
}

StatementHolder::StatementHolder(Connection* con) :
    _con(con),
    _stmt(con->createStatement())
{
}

StatementHolder::StatementHolder(const ConnectionHolderPtr& conh) :
    _con(conh->connection()),
    _stmt(conh->connection()->createStatement())
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
