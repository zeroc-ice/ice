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
    getRef(conh.connection(), decodeName(current.id.name));
    conh.commit();
}


HR::EmpDesc 
EmpI::getDesc(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    HR::EmpDesc result;
    {
	Ref<EMP_T> empRef = getRef(conh.connection(), decodeName(current.id.name));
    
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
	    deptId.name = encodeName(deptRef->getDeptno());
	    deptId.category = _deptCategory;
	    result.edept = 
		HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
	}
    }
    conh.commit();
    return result;
}
    
void EmpI::updateDesc(const HR::EmpDesc& newDesc, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    {
	Ref<EMP_T> empRef = getRef(conh.connection(), decodeName(current.id.name));
   
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
	    empRef->setMgrref(mgrRef);
	}
	
	if(newDesc.hiredate == "")
	{
	    empRef->setHiredate(Date());
	}
	else
	{
	    Date hiredate(_env);
	    hiredate.fromText(newDesc.hiredate);
	    empRef->setHiredate(hiredate);
	}
	
	if(newDesc.sal == "")
	{
	    empRef->setSal(Number());
	}
	else
	{
	    Number sal(0);
	    sal.fromText(_env, newDesc.sal, "99999.99");
	    empRef->setSal(sal);
	}
	
	if(newDesc.comm == "")
	{
	    empRef->setComm(Number());	
	}
	else
	{
	    Number comm(0);
	    comm.fromText(_env, newDesc.comm, "0.999");
	    empRef->setComm(comm);
	}
	
	if(newDesc.edept == 0)
	{
	    empRef->setDeptref(Ref<DEPT_T>());
	}
	else
	{
	    Ref<DEPT_T> deptRef = DeptI::getRef(conh.connection(), 
						decodeName(newDesc.edept->ice_getIdentity().name));
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
	Ref<EMP_T> empRef = getRef(conh.connection(), decodeName(current.id.name));
	empRef->markDelete();
    }
    conh.commit();
}

/*static*/
Ref<EMP_T> 
EmpI::getRef(Connection* con, int empno)
{
    StatementHolder stmth(con);
    
    stmth.statement()->setSQL("SELECT REF(e) FROM EMP_VIEW e WHERE EMPNO = :1");
    stmth.statement()->setInt(1, empno);
    auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());
    
    if(rs->next() == ResultSet::END_OF_FETCH)
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }
   
    return rs->getRef(1);
}
