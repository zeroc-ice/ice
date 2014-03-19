// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DEPTI_H
#define DEPTI_H

#include <Ice/Ice.h>
#include <DeptFactoryI.h>
#include <HR.h>
#include <occi.h>
#include <DbTypes.h>
#include <Util.h>

class DeptI : public HR::Dept
{
public:

    DeptI(const oracle::occi::RefAny&, const ConnectionHolderPtr&, const DeptFactoryIPtr&);
    
    virtual HR::EmpPrx createEmp(int, const HR::EmpDesc&, const Ice::Current&);

    virtual HR::DeptDesc getDesc(const Ice::Current&);    
    virtual void updateField(const std::string&, const std::string&, const Ice::Current&);
    virtual void remove(const Ice::Current&);

    virtual HR::EmpPrxSeq findAll(const Ice::Current&);
    virtual HR::EmpPrxSeq findByName(const std::string&, const Ice::Current&);

private:

    const oracle::occi::Ref<DEPT_T> _ref;
    const ConnectionHolderPtr _conh;
    const DeptFactoryIPtr _factory;
};

#endif
