// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DEPT_FACTORYI_H
#define DEPT_FACTORYI_H

#include <HR.h>
#include <CurrentSqlContext.h>

class DeptFactoryI : public HR::DeptFactory
{
public:

    DeptFactoryI(const CurrentSqlContext&, const std::string&, const std::string&);
    
    virtual HR::DeptPrx createDept(int, const HR::DeptDesc&, const Ice::Current&);

    virtual HR::DeptPrxSeq findAll(const Ice::Current&);
    virtual HR::DeptPrxSeq findByName(const std::string&, const Ice::Current&);
    
    virtual HR::DeptPrx findDeptByNo(int, const Ice::Current&);
    virtual HR::EmpPrx findEmpByNo(int, const Ice::Current&);

private:
    const CurrentSqlContext _currentCtx;
    const std::string _deptCategory;
    const std::string _empCategory;
};

#endif
