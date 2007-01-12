// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <DeptI.h>
#include <Util.h>
#include <EmpI.h>

#ifdef _MSC_VER
//
// ott generates placement new without the corresponding delete
//
#   pragma warning( 4 : 4291 )
#endif

using namespace std;
using namespace oracle::occi;

DeptI::DeptI(Environment* env, StatelessConnectionPool* pool, const string& empCategory) :
    _env(env),
    _pool(pool),
    _empCategory(empCategory)
{
}

void 
DeptI::ice_ping(const Ice::Current& current) const
{
    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> ref = decodeRef(current.id.name, _env, conh.connection());
	
	try
	{
	    //
	    // Dereferences object to see if it existts
	    //
	    ref.ptr();
	}
	catch(const SQLException& sqle)
	{
	    if(sqle.getErrorCode() == 21700)
	    {
		throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	    }
	    else
	    {
		throw;
	    }
	}
    }
    conh.commit();
}

HR::EmpPrx 
DeptI::createEmp(int empno, const HR::EmpDesc& desc, const Ice::Current& current)
{
    Ice::Identity empId;
    empId.category = _empCategory;

    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> deptRef = decodeRef(current.id.name, _env, conh.connection());

	//
	// Inserted into the OCCI cache
	//
	Ref<EMP_T> emp = new(conh.connection(), "EMP_VIEW")EMP_T;
	
	emp->setEmpno(empno);
	emp->setEname(desc.ename);
	emp->setJob(desc.job);
	if(desc.mgr != 0)
	{
	    Ref<EMP_T> mgrRef = decodeRef(desc.mgr->ice_getIdentity().name, _env, conh.connection());
	    
	    emp->setMgrref(mgrRef);
	}
	
	if(desc.hiredate != "")
	{
	    Date hiredate(_env);
	    hiredate.fromText(desc.hiredate);
	    emp->setHiredate(hiredate);
	}
	
	if(desc.sal != "")
	{
	    Number sal(0);
	    sal.fromText(_env, desc.sal, "99999.99");
	    emp->setSal(sal);
	}
	
	if(desc.comm != "")
	{
	    Number comm(0);
	    comm.fromText(_env, desc.comm, "0.999");
	    emp->setComm(comm);
	}
	
	emp->setDeptref(deptRef);

	empId.name = encodeRef(emp, _env);
    }

    conh.commit();
    return HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId));
}

HR::DeptDesc 
DeptI::getDesc(const Ice::Current& current)
{
    assert(this != 0);
    HR::DeptDesc result;
    
    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> deptRef = decodeRef(current.id.name, _env, conh.connection());
	result.deptno = deptRef->getDeptno();
	result.dname = deptRef->getDname();
	result.loc = deptRef->getLoc();
    }
    conh.commit();
    return result;
}
    
void DeptI::updateField(const string& field, const string& newValue, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> deptRef = decodeRef(current.id.name, _env, conh.connection());
	    
	if(field == "dname")
	{
	    deptRef->setDname(newValue);
	}
	else if(field == "loc")
	{
	    deptRef->setLoc(newValue);
	}
	else
	{
	    throw HR::SqlException("There is no field " + field + " in object DEPT_T");
	}
	deptRef->markModified();
    }
    conh.commit();
}

void DeptI::remove(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> deptRef = decodeRef(current.id.name, _env, conh.connection());
	deptRef->markDelete();
    }
    conh.commit();
}

HR::EmpPrxSeq 
DeptI::findAll(const Ice::Current& current)
{
    HR::EmpPrxSeq result;

    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> deptRef = decodeRef(current.id.name, _env, conh.connection());

	StatementHolder stmth(conh);
	stmth.statement()->setSQL("SELECT REF(e) FROM EMP_VIEW e WHERE DEPTREF = :1");
	stmth.statement()->setRef(1, deptRef);
	auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());
	
	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity empId;
	    empId.category = _empCategory;
	    empId.name = encodeRef(rs->getRef(1), _env);
	    
	    result.push_back(HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId)));
	}
    }
    conh.commit();
    return result;
}


HR::EmpPrxSeq 
DeptI::findByName(const string& name, const Ice::Current& current)
{
    HR::EmpPrxSeq result;

    ConnectionHolder conh(_pool);
    {
	Ref<DEPT_T> deptRef = decodeRef(current.id.name, _env, conh.connection());	

	StatementHolder stmth(conh);
	stmth.statement()->setSQL("SELECT REF(e) FROM EMP_VIEW e WHERE DEPTREF = :1 AND ENAME = :2");
	stmth.statement()->setRef(1, deptRef);
	stmth.statement()->setString(2, name);
	auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());
	
	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity empId;
	    empId.category = _empCategory;
	    empId.name = encodeRef(rs->getRef(1), _env);
	    
	    result.push_back(HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId)));
	}
    }
    conh.commit();
    return result;
}
