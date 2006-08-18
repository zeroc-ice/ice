// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <DeptFactoryI.h>
#include <Util.h>
#include <DbTypes.h>

#ifdef _MSC_VER
//
// ott generates placement new without the corresponding delete
//
#   pragma warning( 4 : 4291 )
#endif

using namespace std;
using namespace oracle::occi;

DeptFactoryI::DeptFactoryI(StatelessConnectionPool* pool, 
			   const string& deptCategory) :
    _pool(pool),
    _deptCategory(deptCategory)
{
}

HR::DeptPrx 
DeptFactoryI::createDept(int deptno, const HR::DeptDesc& desc, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);

    //
    // Inserted into the OCCI cache
    //
    DEPT_T* dept = new(conh.connection(), "DEPT_VIEW")DEPT_T;
    dept->setDeptno(deptno);
    dept->setDname(desc.dname);
    dept->setLoc(desc.loc);
    conh.commit();

    Ice::Identity deptId;
    deptId.name = encodeName(deptno);
    deptId.category = _deptCategory;
    return HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
}

HR::DeptPrxSeq 
DeptFactoryI::findAll(const Ice::Current& current)
{
    HR::DeptPrxSeq result;

    ConnectionHolder conh(_pool);
    {
	StatementHolder stmth(conh);
    
	auto_ptr<ResultSet> rs(
	    stmth.statement()->executeQuery("SELECT DEPTNO FROM DEPT_VIEW"));
	
	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity deptId;
	    deptId.category = _deptCategory;
	    deptId.name = rs->getString(1); // first column as string
	    
	    result.push_back(
		HR::DeptPrx::uncheckedCast(
		    current.adapter->createProxy(deptId)));
	}
    }
    conh.commit();
    return result;
}


HR::DeptPrxSeq 
DeptFactoryI::findByName(const string& name, const Ice::Current& current)
{
    HR::DeptPrxSeq result;

    ConnectionHolder conh(_pool);
    {
	StatementHolder stmth(conh);
	stmth.statement()->setSQL("SELECT DEPTNO FROM DEPT_VIEW WHERE DNAME = :1");
	stmth.statement()->setString(1, name);
	stmth.statement()->execute();
	
	auto_ptr<ResultSet> rs(
	    stmth.statement()->getResultSet());

	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity deptId;
	    deptId.category = _deptCategory;
	    deptId.name = rs->getString(1); // first column as string
	    
	    result.push_back(
		HR::DeptPrx::uncheckedCast(
		    current.adapter->createProxy(deptId)));
	}
    }
    conh.commit();
    return result;
}
