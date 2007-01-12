// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <EmpI.h>
#include <DeptI.h>
#include <Util.h>
#include <occi.h>

using namespace std;
using namespace oracle::occi;

EmpI::EmpI(DeptFactoryIPtr factory, Environment* env, StatelessConnectionPool* pool,
	   const string& empCategory, const string& deptCategory) :
    _factory(factory), _env(env), _pool(pool), 
    _empCategory(empCategory), _deptCategory(deptCategory)
{
}

void 
EmpI::ice_ping(const Ice::Current& current) const
{
    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> ref = decodeRef(current.id.name, _env, conh.connection());
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


HR::EmpDesc 
EmpI::getDesc(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    HR::EmpDesc result;
    {
	Ref<EMP_T> empRef = decodeRef(current.id.name, _env, conh.connection());
    
	result.empno = empRef->getEmpno();
	result.ename = empRef->getEname();
	result.job = empRef->getJob();
	
	Ref<EMP_T> mgrRef = empRef->getMgrref();
	if(!mgrRef.isNull())
	{
	    Ice::Identity mgrId;
	    mgrId.name = encodeRef(mgrRef, _env);
	    mgrId.category = _empCategory;
	    result.mgr = HR::EmpPrx::uncheckedCast(current.adapter->createProxy(mgrId));
	}
	
	result.hiredate = empRef->getHiredate().toText();

	if(!empRef->getSal().isNull())
	{
	    result.sal = empRef->getSal().toText(_env, "99999.99");
	}

	if(!empRef->getComm().isNull())
	{
	    result.comm = empRef->getComm().toText(_env, "0.999");
	}
	
	Ref<DEPT_T> deptRef = empRef->getDeptref();
	if(!deptRef.isNull())
	{
	    Ice::Identity deptId;
	    deptId.name = encodeRef(deptRef, _env);
	    deptId.category = _deptCategory;
	    result.edept = HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
	}
    }
    conh.commit();
    return result;
}
    
void EmpI::updateField(const string& field, const string& newValue, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> empRef = decodeRef(current.id.name, _env, conh.connection());
   
	if(field == "ename")
	{
	    empRef->setEname(newValue);
	}
	else if(field == "job")
	{
	    empRef->setJob(newValue);
	}
	else if(field == "hiredate")
	{
	    if(newValue == "")
	    {
		empRef->setHiredate(Date());
	    }
	    else
	    {
		Date hiredate(_env);
		hiredate.fromText(newValue);
		empRef->setHiredate(hiredate);
	    }
	}
	else if(field == "sal")
	{
	    if(newValue == "")
	    {
		empRef->setSal(Number());
	    }
	    else
	    {
		Number sal(0);
		sal.fromText(_env, newValue, "99999.99");
		empRef->setSal(sal);
	    }
	}
	else if(field == "comm")
	{
	    if(newValue == "")
	    {
		empRef->setComm(Number());	
	    }
	    else
	    {
		Number comm(0);
		comm.fromText(_env, newValue, "0.999");
		empRef->setComm(comm);
	    }
	}
	else
	{
	    throw HR::SqlException("There is no field " + field + " in type EMP_T");
	}	
	empRef->markModified();
    }
    conh.commit();
}

void EmpI::updateMgr(int newMgr, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> empRef = decodeRef(current.id.name, _env, conh.connection());

	if(newMgr == 0)
	{
	    empRef->setMgrref(Ref<EMP_T>());
	}
	else
	{
	    Ref<EMP_T> mgrRef = _factory->findEmpRefByNo(newMgr, conh.connection());

	    if(mgrRef.isNull())
	    {
		throw HR::SqlException("There is no employee with this empno");
	    }
	    empRef->setMgrref(mgrRef);
	}
	empRef->markModified();
    }
    conh.commit();
}

void EmpI::updateDept(int newDept, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> empRef = decodeRef(current.id.name, _env, conh.connection());

	if(newDept == 0)
	{
	    empRef->setDeptref(Ref<DEPT_T>());
	}
	else
	{
	    Ref<DEPT_T> deptRef = _factory->findDeptRefByNo(newDept, conh.connection());

	    if(deptRef.isNull())
	    {
		throw HR::SqlException("There is no department with this deptno");
	    }
	    empRef->setDeptref(deptRef);
	}
	empRef->markModified();
    }
    conh.commit();
}

void EmpI::remove(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> empRef = decodeRef(current.id.name, _env, conh.connection());
	empRef->markDelete();
    }
    conh.commit();
}
