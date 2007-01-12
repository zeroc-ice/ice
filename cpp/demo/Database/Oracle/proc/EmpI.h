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

#include <HR.h>
#include <CurrentSqlContext.h>

class EmpI : public HR::Emp
{
public:

    EmpI(const CurrentSqlContext&, const std::string&, const std::string&);
    
    virtual void ice_ping(const Ice::Current&) const;

    virtual HR::EmpDesc getDesc(const Ice::Current&);    
    virtual void updateField(const std::string&, const std::string&, const Ice::Current&);
    virtual void updateMgr(int, const Ice::Current&);
    virtual void updateDept(int, const Ice::Current&);
    virtual void remove(const Ice::Current&);

private:
    const CurrentSqlContext _currentCtx;
    const std::string _empCategory;
    const std::string _deptCategory;
};

#endif
