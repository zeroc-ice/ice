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

#include <Ice/Ice.h>
#include <DeptFactoryI.h>
#include <HR.h>
#include <occi.h>
#include <DbTypes.h>
#include <Util.h>

class EmpI : public HR::Emp
{
public:

    EmpI(const oracle::occi::RefAny&, const ConnectionHolderPtr&, const DeptFactoryIPtr&);
  
    virtual HR::EmpDesc getDesc(const Ice::Current&);
    virtual void updateField(const std::string&, const std::string&, const Ice::Current&);
    virtual void updateMgr(int, const Ice::Current&);
    virtual void updateDept(int, const Ice::Current&);
  
    virtual void remove(const Ice::Current&);

private:
    
    const oracle::occi::Ref<EMP_T> _ref;
    const ConnectionHolderPtr _conh;
    const DeptFactoryIPtr _factory;
};

#endif
