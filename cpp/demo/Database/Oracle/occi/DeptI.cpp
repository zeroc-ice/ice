// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
	StatementHolder stmth(conh);
	stmth.statement()->setSQL("BEGIN; SELECT COUNT(*) INTO :1 FROM DEPT_VIEW WHERE DEPTNO = :2; END");
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

HR::EmpPrx 
DeptI::createEmp(int empno, const HR::EmpDesc& desc, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<DEPT_T> deptRef = getRef(conh.connection(), decodeName(current.id.name));
    if(deptRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    //
    // Inserted into the OCCI cache
    //
    EMP_T* emp = new(conh.connection(), "EMP_VIEW")EMP_T;
    
    emp->setEmpno(empno);
    emp->setEname(desc.ename);
    emp->setJob(desc.job);
    if(desc.mgr != 0)
    {
	Ref<EMP_T> mgrRef = 
	    EmpI::getRef(conh.connection(), decodeName(desc.mgr->ice_getIdentity().name));

	if(mgrRef.isNull())
	{
	    throw Ice::ObjectNotExistException(__FILE__, __LINE__);
	}
	emp->setMgrref(mgrRef);
    }

    Date hiredate;
    hiredate.fromText(desc.hiredate);
    emp->setHiredate(hiredate);

    Number sal;
    sal.fromText(_env, desc.sal, "9,999,999.99");
    emp->setSal(sal);

    Number comm;
    comm.fromText(_env, desc.comm, "0.999");
    emp->setComm(comm);

    emp->setDeptref(deptRef);
    conh.commit();

    Ice::Identity empId;
    empId.name = encodeName(empno);
    empId.category = _empCategory;
    return HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId));
}

HR::DeptDesc 
DeptI::getDesc(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<DEPT_T> deptRef = getRef(conh.connection(), decodeName(current.id.name));
    if(deptRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    HR::DeptDesc result = {deptRef->getDname(), deptRef->getLoc()};
    conh.commit();
    return result;
}
    
void DeptI::updateDesc(const HR::DeptDesc& newDesc, const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<DEPT_T> deptRef = getRef(conh.connection(), decodeName(current.id.name));
    if(deptRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    deptRef->setDname(newDesc.dname);
    deptRef->setLoc(newDesc.loc);
    deptRef->markModified();
    conh.commit();
}

void DeptI::remove(const Ice::Current& current)
{
    ConnectionHolder conh(_pool);
    
    Ref<DEPT_T> deptRef = getRef(conh.connection(), decodeName(current.id.name));
    if(deptRef.isNull())
    {
	throw Ice::ObjectNotExistException(__FILE__, __LINE__);
    }

    deptRef->markDelete();
    conh.commit();
}

HR::EmpPrxSeq 
DeptI::findAll(const Ice::Current& current)
{
    HR::EmpPrxSeq result;

    ConnectionHolder conh(_pool);
    {
	StatementHolder stmth(conh);
    
	auto_ptr<ResultSet> rs =
	    stmth.statement()->executeQuery("SELECT EMPNO FROM EMP_VIEW");
	
	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity empId;
	    empId.category = _empCategory;
	    empId.name = rs->getString(1); // first column as string
	    
	    result.push_back(
		HR::EmpPrx::uncheckedCast(
		    current.adapter->createProxy(empId)));
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
	StatementHolder stmth(conh);
	stmth.statement()->setSQL("SELECT EMPNO FROM EMP_VIEW WHERE ENAME = :1");
	stmth.statement()->setString(1, name);
	stmth.statement()->execute();
	
	auto_ptr<ResultSet> rs =
	    stmth.statement()->getResultSet();

	while(rs->next() != ResultSet::END_OF_FETCH)
	{
	    Ice::Identity empId;
	    empId.category = _empCategory;
	    empId.name = rs->getString(1); // first column as string
	    
	    result.push_back(
		HR::EmpPrx::uncheckedCast(
		    current.adapter->createProxy(empId)));
	}
    }
    conh.commit();
    return result;
}


/*static*/
Ref<DEPT_T> 
DeptI::getRef(Connection* con, int deptno)
{
    StatementHolder stmth(con);
    stmth.statement()->setSQL("BEGIN; SELECT REF(d) INTO :1 FROM DEPT_VIEW d WHERE DEPTNO = :2; END");
    stmth.statement()->registerOutParam(1, OCCIREF);
    stmth.statement()->setInt(2, deptno);
    stmth.statement()->execute();
    return stmth.statement()->getRef(1);
}
