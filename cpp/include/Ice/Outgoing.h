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

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ConnectionF.h>
#include <Ice/ReferenceF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace Ice
{

class LocalException;

}

namespace IceInternal
{

//
// An exception wrapper, which is used to indicate that an operation
// that failed due to an exception is not repeatable if "at-most-once"
// semantics must be guaranteed.
//
class ICE_API NonRepeatable
{
public:

    NonRepeatable(const NonRepeatable&);
    NonRepeatable(const ::Ice::LocalException&);
    const ::Ice::LocalException* get() const;

private:

    const NonRepeatable& operator=(const NonRepeatable&);

    std::auto_ptr< ::Ice::LocalException> _ex;
};

class ICE_API Outgoing : public ::IceUtil::noncopyable, public IceUtil::Monitor< IceUtil::Mutex >
{
public:

    Outgoing(const ConnectionPtr&, const ReferencePtr&, const std::string&, ::Ice::OperationMode,
	     const ::Ice::Context&);
    ~Outgoing();

    bool invoke(); // Returns true if ok, false if user exception.
    void finished(BasicStream&);
    void finished(const ::Ice::LocalException&);

    BasicStream* is();
    BasicStream* os();

private:

    //
    // Optimization. The connection and the reference may not be deleted
    // while a stack-allocated Incoming still holds it.
    //
    const ConnectionPtr& _connection;
    const ReferencePtr& _reference;

    std::auto_ptr< ::Ice::LocalException> _exception;

    enum
    {
	StateUnsent,
	StateInProgress,
	StateOK,
	StateUserException,
	StateLocalException,
    } _state;

    BasicStream _is;
    BasicStream _os;
};

}

#endif
