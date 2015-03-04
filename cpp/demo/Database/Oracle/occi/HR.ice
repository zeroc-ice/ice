// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

//
// Corresponds to the well-known Dept/Emp Oracle demo database.
// See rdbms/admin/utlsamp.sql
//


module HR
{

interface Dept;
sequence<Dept*> DeptPrxSeq;

interface Emp;
sequence<Emp*> EmpPrxSeq;

exception SqlException
{
    string reason;
};

//
// The data we maintain about each department
//
struct DeptDesc
{
    int deptno;
    string dname;
    string loc;
};

//
// The data we maintain about each Employee
//
struct EmpDesc
{
    int empno;
    string ename;
    string job;
    Emp* mgr;
    string hiredate;
    string sal;
    string comm;
    Dept* edept;
};

interface Emp
{
    idempotent EmpDesc getDesc();
    idempotent void updateField(string name, string newValue) throws SqlException;
    idempotent void updateMgr(int mgr) throws SqlException;
    idempotent void updateDept(int deptno) throws SqlException;
    void remove();
};

interface Dept
{
    Emp* createEmp(int empno, EmpDesc descx) throws SqlException;

    idempotent DeptDesc getDesc();
    idempotent void updateField(string name, string newValue) throws SqlException;
    void remove() throws SqlException;

    idempotent EmpPrxSeq findAll();
    idempotent EmpPrxSeq findByName(string ename);
};

interface DeptFactory
{
    Dept* createDept(int deptno, DeptDesc desc) throws SqlException;

    idempotent DeptPrxSeq findAll();
    idempotent DeptPrxSeq findByName(string dname);

    idempotent Emp* findEmpByNo(int empno);
    idempotent Dept* findDeptByNo(int deptno); 
};

};
