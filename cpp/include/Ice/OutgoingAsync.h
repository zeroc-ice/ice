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

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <Ice/OutgoingAsyncF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ReferenceF.h>
#include <Ice/InstanceF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class BasicStream;

class ICE_API OutgoingAsync : public ::IceUtil::Shared
{
public:

    OutgoingAsync();
    virtual ~OutgoingAsync();

    virtual void ice_exception(const ::IceUtil::Exception&) = 0;
    virtual void __response(bool) = 0;

    void __setup(const ConnectionPtr&, const ReferencePtr&, const std::string&, ::Ice::OperationMode,
		 const ::Ice::Context&);

    void __invoke();
    void __finished(BasicStream&);
    void __finished(const ::Ice::LocalException&);

    BasicStream* __is();
    BasicStream* __os();

private:

    void warning(const ::Ice::Exception&) const;
    void warning(const ::std::exception&) const;
    void warning() const;

    ConnectionPtr _connection;
    bool _compress;

    //
    // We need a separate InstancePtr, because _is and _os only hold a
    // const InstancePtr& for optimization.
    //
    InstancePtr _instance;

    BasicStream* _is;
    BasicStream* _os;
};

}

namespace Ice
{

class ICE_API AMI_Object_ice_invoke : public ::IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const ::std::vector< ::Ice::Byte>&) = 0;
    virtual void ice_exception(const ::IceUtil::Exception&) = 0;

private:

    virtual void __response(bool);
};

}

#endif
