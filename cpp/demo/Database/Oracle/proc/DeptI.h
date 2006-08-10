// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef DEPTI_H
#define DEPTI_H

#include <HR.h>
#include <CurrentSqlContext.h>

class DeptI : public HR::Dept
{
public:

    DeptI(const CurrentSqlContext&, const std::string&);
    
    virtual void ice_ping(const Ice::Current&);

    virtual HR::EmpPrx createEmp(int, const HR::EmpDesc&, const Ice::Current&);

    virtual HR::DeptDesc getDesc(const Ice::Current&);    
    virtual void updateDesc(const HR::DeptDesc& newDesc, const Ice::Current&);
    virtual void remove(const Ice::Current&);

    virtual HR::EmpPrxSeq findAll(const Ice::Current&);
    virtual HR::EmpPrxSeq findByName(const std::string&, const Ice::Current&);

private:
    const CurrentSqlContext _currentCtx;
    const std::string _empCategory;
};

#endif
