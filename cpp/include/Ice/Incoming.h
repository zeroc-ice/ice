// **********************************************************************
//
// Copyright (c) 2003
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

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/InstanceF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/ServantManagerF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace IceInternal
{

class ICE_API IncomingBase : public IceUtil::noncopyable
{
protected:

    IncomingBase(Instance*, Connection*, const Ice::ObjectAdapterPtr&, bool, Ice::Byte);
    IncomingBase(IncomingBase& in); // Adopts the argument. It must not be used afterwards.
    
    void __finishInvoke();
    void __warning(const Ice::Exception&) const;
    void __warning(const std::string&) const;

    Ice::Current _current;
    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
    Ice::LocalObjectPtr _cookie;

    bool _response;
    Ice::Byte _compress;

    BasicStream _os;

//
// Cannot be private. IncomingAsync needs _connection to initialize a
// ConnectionPtr.
//
//private:

    //
    // Optimization. The connection may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    Connection* _connection;
};

class ICE_API Incoming : public IncomingBase
{
public:

    Incoming(Instance*, Connection*, const Ice::ObjectAdapterPtr&, bool, Ice::Byte);

    void invoke(const ServantManagerPtr&);

    // Inlined for speed optimization.
    BasicStream* is() { return &_is; }
    BasicStream* os() { return &_os; }

private:

    BasicStream _is;
};

}

#endif
