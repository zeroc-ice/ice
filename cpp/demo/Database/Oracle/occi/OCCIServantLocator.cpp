// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <OCCIServantLocator.h>
#include <EmpI.h>
#include <DeptI.h>
#include <DbTypes.h>
#include <Util.h>
#include <occi.h>

using namespace std;
using namespace oracle::occi;

OCCIServantLocator::OCCIServantLocator(const DeptFactoryIPtr& factory) :
    _factory(factory)
{}


Ice::ObjectPtr 
OCCIServantLocator::locate(const Ice::Current& current, Ice::LocalObjectPtr& cookie)
{
    ConnectionHolderPtr con = new ConnectionHolder(_factory->getConnectionPool());
    RefAny ref = decodeRef(current.id.name, _factory->getEnv(), con->connection());

    try
    {
        //
        // Extract SQL type from target object
        //
        string sqlType = Ref<PObject>(ref)->getSQLTypeName();

        //
        // Create and return the servant, used only for this one operation
        //
        if(sqlType.find("EMP_T") != string::npos)
        {
            return new EmpI(ref, con, _factory);
        }
        else if(sqlType.find("DEPT_T") != string::npos)
        {
            return new DeptI(ref, con, _factory);
        }
        else
        {
            return 0;
        }
    }
    catch(const SQLException& sqle)
    {
        if(sqle.getErrorCode() == 21700)
        {
            return 0;
        }
        else
        {
            throw;
        }
    }
}

void 
OCCIServantLocator::finished(const Ice::Current&, const Ice::ObjectPtr&, const Ice::LocalObjectPtr&)
{
    //
    // Nothing to do: if the connection was not yet released, it is released (and the tx rolled back)
    // when the servant's last refcount goes away
    //
}

void 
OCCIServantLocator::deactivate(const string&)
{
}

