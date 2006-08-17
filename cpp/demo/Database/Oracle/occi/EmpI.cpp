// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <EmpI.h>
#include <Util.h>
#include <occi.h>

using namespace std;
using namespace oracle::occi;

EmpI::EmpI(Environment* env, StatelessConnectionPool* pool,
	   const string& empCategory, const string& deptCategory) :
    _env(env),
    _pool(pool), 
    _empCategory(empCategory), _deptCategory(deptCategory)
{
}

void 
EmpI::ice_ping(const Ice::Current& current) const
{
    ConnectionHolder conh(_pool);
    {
	StatementHolder stmth(conh);
      
	stmth.statement()->setSQL("BEGIN; SELECT COUNT(*) INTO :1 FROM EMP_VIEW WHERE EMPNO = :2; END");
	stmth.statement()->registerOutParam(1, OCCIINT);
	stmth.statement()->setString(2, current.id.name);
	stmth.statement()->execute();
	
	if(stmth.statement()->getInt(1) == 0)
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
    }
    conh.commit();
}


HR::EmpDesc 
EmpI::getDesc(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<EMP_T> empRef = getRef(conh.connection(), decodeName(current.id.name));
    if(empRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    HR::EmpDesc result;
    result.ename = empRef->getEname();
    result.job = empRef->getJob();
    
    Ref<EMP_T> mgrRef = empRef->getMgrref();
    if(!mgrRef.isNull())
    {
	Ice::Identity mgrId;
	mgrId.name = encodeName(mgrRef->getEmpno());
	mgrId.category = _empCategory;
	result.mgr = 
	    HR::EmpPrx::uncheckedCast(current.adapter->createProxy(mgrId));
    }
    
    if(!empRef->getSal().isNull())
    {
	result.sal = empRef->getSal().toText(_env, "9,999,999.99");
    }
    
    if(!empRef->getComm().isNull())
    {
	result.comm = empRef->getComm().toText(_env, "0.999");
    }
    
    Ref<DEPT_T> deptRef = empRef->getDeptref();
    if(!deptRef.isNull())
    {
	Ice::Identity deptId;
	deptId.name = encodeName(deptRef->getDeptno());
	deptId.category = _deptCategory;
	result.edept = 
	    HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
    }

    conh.commit();
    return result;
}
    
void EmpI::updateDesc(const HR::EmpDesc& newDesc, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<EMP_T> empRef = getRef(conh.connection(), decodeName(current.id.name));
    if(empRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    empRef->setEname(newDesc.ename);
    empRef->setJob(newDesc.job);
    
    if(newDesc.mgr == 0)
    {
	empRef->setMgrref(Ref<EMP_T>());
    }
    else
    {
	Ref<EMP_T> mgrRef = getRef(conh.connection(), 
				   decodeName(newDesc.mgr->ice_getIdentity().name));
	if(mgrRef.isNull())
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	empRef->setMgrref(mgrRef);
    }

    Date hiredate;
    hiredate.fromText(newDesc.hiredate);
    empRef->setHiredate(hiredate);

    Number sal;
    sal.fromText(_env, newDesc.sal, "9,999,999.99");
    empRef->setSal(sal);

    Number comm;
    comm.fromText(_env, newDesc.comm, "0.999");
    empRef->setComm(comm);

    if(newDesc.edept == 0)
    {
	empRef->setDeptref(Ref<DEPT_T>());
    }
    else
    {
	Ref<DEPT_T> deptRef = getRef(conh.connection(), 
				     decodeName(newDesc.edept->ice_getIdentity().name));
	if(deptRef.isNull())
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	empRef->setDeptref(deptRef);
    }

    empRef->markModified();
    conh.commit();
}

void EmpI::remove(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<EMP_T> empRef = getRef(conh.connection(), decodeName(current.id.name));
    if(empRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    empRef->markDelete();
    conh.commit();
}

/*static*/
Ref<EMP_T> 
EmpI::getRef(Connection* con, int empno)
{
    StatementHolder stmth(con);
    stmth.statement()->setSQL("BEGIN; SELECT REF(e) INTO :1 FROM EMP_VIEW e WHERE EMPNO = :2; END");
    stmth.statement()->registerOutParam(1, OCCIREF);
    stmth.statement()->setInt(2, empno);
    stmth.statement()->execute();
    return stmth.statement()->getRef(1);
}
