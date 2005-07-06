// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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

class ICEE_API IncomingBase : private Ice::noncopyable
{
protected:

    IncomingBase(Instance*, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool);
    IncomingBase(IncomingBase& in); // Adopts the argument. It must not be used afterwards.
    
    void __warning(const Ice::Exception&) const;
    void __warning(const std::string&) const;

    Ice::Current _current;
    Ice::ObjectPtr _servant;
    Ice::LocalObjectPtr _cookie;

    bool _response;

    BasicStream _os;

    //
    // Optimization. The connection may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    Ice::Connection* _connection;
};

class ICEE_API Incoming : public IncomingBase
{
public:

    Incoming(Instance*, Ice::Connection*, const Ice::ObjectAdapterPtr&, bool);

    void invoke(const ServantManagerPtr&);

    // Inlined for speed optimization.
    BasicStream* is() { return &_is; }
    BasicStream* os() { return &_os; }

private:

    BasicStream _is;
};

}

#endif
