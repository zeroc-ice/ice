// **********************************************************************
//
// Copyright (c) 2001
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ADMIN_I_H
#define ADMIN_I_H

#include <Yellow/Yellow.h>
#include <Yellow/StringObjectProxySeqDict.h>

namespace Yellow
{

class AdminI : public Admin, public IceUtil::Mutex
{
public:
    
    AdminI(const Freeze::DBPtr&);

    virtual void add(const ::std::string&, const ::Ice::ObjectPrx&, const Ice::Current&);
    virtual void remove(const ::std::string&, const ::Ice::ObjectPrx&, const Ice::Current&);

private:
    
    StringObjectProxySeqDict _dict;
};

}

#endif
