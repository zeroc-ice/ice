// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    Incoming(Instance*, Ice::Connection*, BasicStream&);

    void invoke(bool, Ice::ObjectAdapter*, ServantManager*);

    // Inlined for speed optimization.
    BasicStream* os() { return &_os; }
    BasicStream* is() { return &_is; }

protected:
    
    void __warning(const Ice::Exception&) const;
    void __warning(const std::string&) const;

    Ice::Current _current;
    bool _response;
    BasicStream _os;
    BasicStream& _is;

    //
    // Optimization. The connection may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    Ice::Connection* _connection;
};

}

#endif
