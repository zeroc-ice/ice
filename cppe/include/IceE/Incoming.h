// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_INCOMING_H
#define ICEE_INCOMING_H

#include <IceE/InstanceF.h>
#include <IceE/ConnectionF.h>
#include <IceE/ServantManagerF.h>
#include <IceE/BasicStream.h>
#include <IceE/Current.h>

namespace IceInternal
{

class ICE_API Incoming : private IceUtil::noncopyable
{
public:

    Incoming(Instance*, Ice::Connection*, BasicStream&, const Ice::ObjectAdapterPtr&);

    void setAdapter(const Ice::ObjectAdapterPtr&);
    const Ice::ObjectAdapterPtr& getAdapter() const { return _adapter; }

    void invoke(bool, Ice::Int);

    // Inlined for speed optimization.
    BasicStream* os() { return &_os; }
    BasicStream* is() { return &_is; }

protected:
    
    void __warning(const Ice::Exception&) const;
    void __warning(const std::string&) const;

    BasicStream _os;
    BasicStream& _is;
    Ice::Connection* _connection;
    Ice::ObjectAdapterPtr _adapter;
    ServantManager* _servantManager;
    Ice::Current _current;
};

}

#endif
