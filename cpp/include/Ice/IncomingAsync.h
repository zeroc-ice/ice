// **********************************************************************
//
// Copyright (c) 2002
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

#ifndef ICE_INCOMING_ASYNC_H
#define ICE_INCOMING_ASYNC_H

#include <Ice/IncomingAsyncF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace IceInternal
{

class Incoming;

class ICE_API IncomingAsync : public ::IceUtil::Shared
{
public:

    IncomingAsync(Incoming&);

protected:

    void __response(bool);
    void __exception(const ::Ice::Exception&);
    void __fatal(const ::Ice::LocalException&);

    BasicStream* __is();
    BasicStream* __os();

private:

    void finishInvoke();

    Ice::Current _current;
    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
    Ice::LocalObjectPtr _cookie;

    ConnectionPtr _connection;
    bool _compress;

    //
    // We need a separate InstancePtr, because _is and _os only hold a
    // const InstancePtr& for optimization.
    //
    InstancePtr _instance;

    BasicStream _is;
    BasicStream _os;
};

}

namespace Ice
{

class ICE_API AMD_Object_ice_invoke : public ::IceInternal::IncomingAsync
{
public:

    AMD_Object_ice_invoke(::IceInternal::Incoming&);
    
    void ice_response(bool, const ::std::vector< ::Ice::Byte>&);
    void ice_exception(const ::IceUtil::Exception&);
};

}

#endif
