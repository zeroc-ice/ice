// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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

DeptFactoryI::DeptFactoryI(Environment* env, StatelessConnectionPool* pool, 
			   const string& empCategory, const string& deptCategory) :
    _env(env),
    _pool(pool),
    _empCategory(empCategory),
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
    Ref<DEPT_T> dept = new(conh.connection(), "DEPT_VIEW")DEPT_T;
    dept->setDeptno(deptno);
    dept->setDname(desc.dname);
    dept->setLoc(desc.loc);
   
    Ice::Identity deptId;
    deptId.name = encodeRef(dept, _env);
    deptId.category = _deptCategory;
    conh.commit();

    return HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
}

HR::DeptPrxSeq 
DeptFactoryI::findAll(const Ice::Current& current)
{
    HR::DeptPrxSeq result;

    ConnectionHolder conh(_pool);
    {
	StatementHolder stmth(conh);
    
	auto_ptr<ResultSet> rs(stmth.statement()->executeQuery("SELECT REF(d) FROM DEPT_VIEW d"));
	
	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity deptId;
	    deptId.category = _deptCategory;
	    deptId.name = encodeRef(rs->getRef(1), _env);
	    
	    result.push_back(HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId)));
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
	stmth.statement()->setSQL("SELECT REF(d) FROM DEPT_VIEW d WHERE DNAME = :1");
	stmth.statement()->setString(1, name);
	
	auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());

	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity deptId;
	    deptId.category = _deptCategory;
	    deptId.name = encodeRef(rs->getRef(1), _env);
	    
	    result.push_back(HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId)));
	}
    }
    conh.commit();
    return result;
}

HR::DeptPrx
DeptFactoryI::findDeptByNo(int deptno, const Ice::Current& current)
{
    HR::DeptPrx result;

    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> ref = findDeptRefByNo(deptno, conh.connection());

	if(!ref.isNull())
	{
	    Ice::Identity deptId;
	    deptId.category = _deptCategory;
	    deptId.name = encodeRef(ref, _env);
	    result = HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
	}
    }
    conh.commit();
    return result;
}

HR::EmpPrx
DeptFactoryI::findEmpByNo(int empno, const Ice::Current& current)
{
    HR::EmpPrx result;

    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> ref = findEmpRefByNo(empno, conh.connection());

	if(!ref.isNull())
	{
	    Ice::Identity empId;
	    empId.category = _empCategory;
	    empId.name = encodeRef(ref, _env);
	    result = HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId));
	}
    }
    conh.commit();
    return result;
}

Ref<DEPT_T>
DeptFactoryI::findDeptRefByNo(int deptno, Connection* con) const
{
    StatementHolder stmth(con);
    stmth.statement()->setSQL("SELECT REF(d) FROM DEPT_VIEW d WHERE DEPTNO = :1");
    stmth.statement()->setInt(1, deptno);
    auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());
    
    if(rs->next() == ResultSet::END_OF_FETCH)
    {
	return Ref<DEPT_T>();
    }
    return rs->getRef(1);
}

Ref<EMP_T>
DeptFactoryI::findEmpRefByNo(int empno, Connection* con) const
{
    StatementHolder stmth(con);
    stmth.statement()->setSQL("SELECT REF(e) FROM EMP_VIEW e WHERE EMPNO = :1");
    stmth.statement()->setInt(1, empno);
    auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());

    if(rs->next() == ResultSet::END_OF_FETCH)
    {
	return Ref<EMP_T>();
    }
    return rs->getRef(1);
}
