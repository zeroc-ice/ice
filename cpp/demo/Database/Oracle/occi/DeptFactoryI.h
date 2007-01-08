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
#include <occi.h>

class DeptFactoryI : public HR::DeptFactory
{
public:

    DeptFactoryI(oracle::occi::StatelessConnectionPool*, const std::string&);
    
    virtual HR::DeptPrx createDept(int, const HR::DeptDesc&, const Ice::Current&);

    virtual HR::DeptPrxSeq findAll(const Ice::Current&);
    virtual HR::DeptPrxSeq findByName(const std::string&, const Ice::Current&);

private:
    oracle::occi::StatelessConnectionPool* _pool;
    const std::string _deptCategory;
};

#endif
