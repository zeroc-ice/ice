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

#ifndef ICE_OUTGOING_ASYNC_H
#define ICE_OUTGOING_ASYNC_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <IceUtil/Time.h>
#include <Ice/OutgoingAsyncF.h>
#include <Ice/ReferenceF.h>
#include <Ice/ConnectionF.h>
#include <Ice/Current.h>

namespace IceInternal
{

class BasicStream;

//
// We need virtual inheritance from shared, because the user might use
// multiple inheritance from IceUtil::Shared.
//
class ICE_API OutgoingAsync : virtual public IceUtil::Shared
{
public:

    OutgoingAsync();
    virtual ~OutgoingAsync();

    virtual void ice_exception(const Ice::Exception&) = 0;

    void __finished(BasicStream&);
    void __finished(const Ice::LocalException&);

    bool __timedOut() const;

protected:

    void __prepare(const IceInternal::ReferencePtr&, const std::string&, Ice::OperationMode, const Ice::Context&);
    void __send();
    void __cleanup();

    virtual void __response(bool) = 0;

    BasicStream* __is;
    BasicStream* __os;

private:

    void warning(const Ice::Exception&) const;
    void warning(const std::exception&) const;
    void warning() const;

    ReferencePtr _reference;
    ConnectionPtr _connection;
    int _cnt;
    Ice::OperationMode _mode;

    IceUtil::Time _absoluteTimeout;

    //
    // We don't want to derive from a mutex, because this would be too
    // intrusive for user code, i.e., classes that derive from this
    // one in order to implement ice_exception() and ice_response().
    //
    IceUtil::Monitor<IceUtil::Mutex> _monitor;

    enum
    {
	StateUnsent,
	StateSent
    } _state;
};

}

namespace Ice
{

class ICE_API AMI_Object_ice_invoke : public IceInternal::OutgoingAsync
{
public:

    virtual void ice_response(bool, const std::vector<Ice::Byte>&) = 0;
    virtual void ice_exception(const Ice::Exception&) = 0;

    void __invoke(const IceInternal::ReferencePtr&, const std::string& operation, OperationMode,
		  const std::vector<Byte>&, const Context&);

private:

    virtual void __response(bool);
};

}

#endif
