// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
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
