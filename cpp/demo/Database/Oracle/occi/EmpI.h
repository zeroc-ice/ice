// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef EMPI_H
#define EMPI_H

#include <DeptFactoryI.h>
#include <HR.h>
#include <occi.h>
#include <DbTypes.h>


class EmpI : public HR::Emp
{
public:

    EmpI(DeptFactoryIPtr factory, oracle::occi::Environment* env,
	 oracle::occi::StatelessConnectionPool*, const std::string&, const std::string&);
    
    virtual void ice_ping(const Ice::Current&) const;

    virtual HR::EmpDesc getDesc(const Ice::Current&);
    virtual void updateField(const std::string&, const std::string&, const Ice::Current&);
    virtual void updateMgr(int, const Ice::Current&);
    virtual void updateDept(int, const Ice::Current&);
  
    virtual void remove(const Ice::Current&);

private:
    DeptFactoryIPtr _factory;
    oracle::occi::Environment* _env;
    oracle::occi::StatelessConnectionPool* _pool;
    const std::string _empCategory;
    const std::string _deptCategory;
};

#endif
