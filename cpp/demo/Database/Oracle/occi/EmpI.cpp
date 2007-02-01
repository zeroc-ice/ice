// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <EmpI.h>

using namespace std;
using namespace oracle::occi;

EmpI::EmpI(const RefAny& ref, const ConnectionHolderPtr& conh, const DeptFactoryIPtr& factory) :
    _ref(ref), _conh(conh), _factory(factory)
{
}

HR::EmpDesc 
EmpI::getDesc(const Ice::Current& current)
{
    Environment* env = _factory->getEnv();
    HR::EmpDesc result;
 
    Ref<EMP_T> empRef = decodeRef(current.id.name, env, _conh->connection());
    
    result.empno = empRef->getEmpno();
    result.ename = empRef->getEname();
    result.job = empRef->getJob();
    
    Ref<EMP_T> mgrRef = empRef->getMgrref();
    if(!mgrRef.isNull())
    {
        Ice::Identity mgrId;
        mgrId.name = encodeRef(mgrRef, env);
        mgrId.category = _factory->getCategory();
        result.mgr = HR::EmpPrx::uncheckedCast(current.adapter->createProxy(mgrId));
    }
    
    result.hiredate = empRef->getHiredate().toText();
    
    if(!empRef->getSal().isNull())
    {
        result.sal = empRef->getSal().toText(env, "99999.99");
    }
    
    if(!empRef->getComm().isNull())
    {
        result.comm = empRef->getComm().toText(env, "0.999");
    }
    
    Ref<DEPT_T> deptRef = empRef->getDeptref();
    if(!deptRef.isNull())
    {
        Ice::Identity deptId;
        deptId.name = encodeRef(deptRef, env);
        deptId.category = _factory->getCategory();
        result.edept = HR::DeptPrx::uncheckedCast(current.adapter->createProxy(deptId));
    }
    _conh->commit();
    return result;
}
    
void 
EmpI::updateField(const string& field, const string& newValue, const Ice::Current& current)
{
    Ref<EMP_T> empRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
    
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
            Date hiredate(_factory->getEnv());
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
            sal.fromText(_factory->getEnv(), newValue, "99999.99");
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
            comm.fromText(_factory->getEnv(), newValue, "0.999");
            empRef->setComm(comm);
        }
    }
    else
    {
        throw HR::SqlException("There is no field " + field + " in type EMP_T");
    }   
    empRef->markModified();
    _conh->commit();
}

void 
EmpI::updateMgr(int newMgr, const Ice::Current& current)
{
    Ref<EMP_T> empRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
    
    if(newMgr == 0)
    {
        empRef->setMgrref(Ref<EMP_T>());
    }
    else
    {
        Ref<EMP_T> mgrRef = _factory->findEmpRefByNo(newMgr, _conh->connection());
        
        if(mgrRef.isNull())
        {
            throw HR::SqlException("There is no employee with this empno");
        }
        empRef->setMgrref(mgrRef);
    }
    empRef->markModified();
    _conh->commit();
}

void 
EmpI::updateDept(int newDept, const Ice::Current& current)
{
    Ref<EMP_T> empRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
    
    if(newDept == 0)
    {
        empRef->setDeptref(Ref<DEPT_T>());
    }
    else
    {
        Ref<DEPT_T> deptRef = _factory->findDeptRefByNo(newDept, _conh->connection());
        
        if(deptRef.isNull())
        {
            throw HR::SqlException("There is no department with this deptno");
        }
        empRef->setDeptref(deptRef);
    }
    empRef->markModified();
    _conh->commit();
}

void 
EmpI::remove(const Ice::Current& current)
{
    Ref<EMP_T> empRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
    empRef->markDelete();
    _conh->commit();
}
