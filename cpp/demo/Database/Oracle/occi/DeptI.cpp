// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <DeptI.h>

#ifdef _MSC_VER
//
// ott generates placement new without the corresponding delete
//
#   pragma warning( 4 : 4291 )
#endif

using namespace std;
using namespace oracle::occi;

DeptI::DeptI(const RefAny& ref, const ConnectionHolderPtr& conh, const DeptFactoryIPtr& factory) :
    _ref(ref), _conh(conh), _factory(factory)
{
}


HR::EmpPrx 
DeptI::createEmp(int empno, const HR::EmpDesc& desc, const Ice::Current& current)
{
    Environment* env = _factory->getEnv();

    //
    // Inserted into the OCCI cache
    //
    EMP_T* emp = new(_conh->connection(), "EMP_VIEW")EMP_T;
        
    emp->setEmpno(empno);
    emp->setEname(desc.ename);
    emp->setJob(desc.job);
    if(desc.mgr != 0)
    {
        Ref<EMP_T> mgrRef = decodeRef(desc.mgr->ice_getIdentity().name, env, _conh->connection());
        
        emp->setMgrref(mgrRef);
    }
    
    if(desc.hiredate != "")
    {
        Date hiredate(env);
        hiredate.fromText(desc.hiredate);
        emp->setHiredate(hiredate);
    }
    
    if(desc.sal != "")
    {
        Number sal(0);
        sal.fromText(env, desc.sal, "99999.99");
        emp->setSal(sal);
    }
    
    if(desc.comm != "")
    {
        Number comm(0);
        comm.fromText(env, desc.comm, "0.999");
        emp->setComm(comm);
    }
    
    emp->setDeptref(_ref);
    
    Ice::Identity empId;
    empId.category = _factory->getCategory();
    empId.name = encodeRef(emp->getRef(), env);
    _conh->commit();
    return HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId));
}

HR::DeptDesc 
DeptI::getDesc(const Ice::Current& current)
{
    HR::DeptDesc result;
   
    Ref<DEPT_T> deptRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
    result.deptno = deptRef->getDeptno();
    result.dname = deptRef->getDname();
    result.loc = deptRef->getLoc();
   
    _conh->commit();
    return result;
}
    
void 
DeptI::updateField(const string& field, const string& newValue, const Ice::Current& current)
{
    Ref<DEPT_T> deptRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
            
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
    _conh->commit();
}

void 
DeptI::remove(const Ice::Current& current)
{
    Ref<DEPT_T> deptRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());
    deptRef->markDelete();
    _conh->commit();
}

HR::EmpPrxSeq 
DeptI::findAll(const Ice::Current& current)
{
    HR::EmpPrxSeq result;
    {
        Ref<DEPT_T> deptRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());

        StatementHolder stmth(_conh);
        stmth.statement()->setSQL("SELECT REF(e) FROM EMP_VIEW e WHERE DEPTREF = :1");
        stmth.statement()->setRef(1, deptRef);
        auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());
        
        while(rs->next() != ResultSet::END_OF_FETCH)
        {
            Ice::Identity empId;
            empId.category = _factory->getCategory();
            empId.name = encodeRef(rs->getRef(1), _factory->getEnv());
            result.push_back(HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId)));
        }
    }
    _conh->commit();
    return result;
}

HR::EmpPrxSeq 
DeptI::findByName(const string& name, const Ice::Current& current)
{
    HR::EmpPrxSeq result;
    {
        Ref<DEPT_T> deptRef = decodeRef(current.id.name, _factory->getEnv(), _conh->connection());      

        StatementHolder stmth(_conh);
        stmth.statement()->setSQL("SELECT REF(e) FROM EMP_VIEW e WHERE DEPTREF = :1 AND ENAME = :2");
        stmth.statement()->setRef(1, deptRef);
        stmth.statement()->setString(2, name);
        auto_ptr<ResultSet> rs(stmth.statement()->executeQuery());
        
        while(rs->next() != ResultSet::END_OF_FETCH)
        {
            Ice::Identity empId;
            empId.category = _factory->getCategory();
            empId.name = encodeRef(rs->getRef(1), _factory->getEnv());
            
            result.push_back(HR::EmpPrx::uncheckedCast(current.adapter->createProxy(empId)));
        }
    }
    _conh->commit();
    return result;
}
