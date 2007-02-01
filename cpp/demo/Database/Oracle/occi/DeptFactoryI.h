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
#include <DbTypes.h>

class DeptFactoryI : public HR::DeptFactory
{
public:

    DeptFactoryI(oracle::occi::Environment* env, oracle::occi::StatelessConnectionPool*, 
                 const std::string&);
    
    virtual HR::DeptPrx createDept(int, const HR::DeptDesc&, const Ice::Current&);

    virtual HR::DeptPrxSeq findAll(const Ice::Current&);
    virtual HR::DeptPrxSeq findByName(const std::string&, const Ice::Current&);

    virtual HR::DeptPrx findDeptByNo(int, const Ice::Current&);
    virtual HR::EmpPrx findEmpByNo(int, const Ice::Current&);

    oracle::occi::Ref<DEPT_T> findDeptRefByNo(int, oracle::occi::Connection*) const;
    oracle::occi::Ref<EMP_T> findEmpRefByNo(int, oracle::occi::Connection*) const;
    
    oracle::occi::Environment* getEnv() const
    {
        return _env;
    }
    
    oracle::occi::StatelessConnectionPool* getConnectionPool() const
    {
        return _pool;
    }
    
    const std::string& getCategory() const
    {
        return _category;
    }

private:
    oracle::occi::Environment* _env;
    oracle::occi::StatelessConnectionPool* _pool;
    const std::string _category;
};

typedef IceUtil::Handle<DeptFactoryI> DeptFactoryIPtr;

#endif
