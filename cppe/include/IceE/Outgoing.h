// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_OUTGOING_H
#define ICEE_OUTGOING_H

#include <IceE/ConnectionF.h>
#include <IceE/ReferenceF.h>

#include <IceE/Mutex.h>
#include <IceE/Monitor.h>
#include <IceE/BasicStream.h>
#include <IceE/OperationMode.h>

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
    NonRepeatable(const Ice::LocalException&);
    const Ice::LocalException* get() const;

private:

    const NonRepeatable& operator=(const NonRepeatable&);

    std::auto_ptr<Ice::LocalException> _ex;
};

class ICE_API Outgoing : private IceUtil::noncopyable
{
public:

    Outgoing(Ice::Connection*, Reference*, const std::string&, Ice::OperationMode, const Ice::Context&);

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);
#ifndef ICEE_PURE_BLOCKING_CLIENT
    void finished(BasicStream&);
#endif
    void finished(const Ice::LocalException&);

    // Inlined for speed optimization.
    BasicStream* is() { return &_is; }
    BasicStream* os() { return &_os; }

private:

    void finishedInternal();

    //
    // Optimization. The connection and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    Ice::Connection* _connection;
    Reference* _reference;

    std::auto_ptr<Ice::LocalException> _exception;
#ifndef ICEE_PURE_BLOCKING_CLIENT
    std::auto_ptr<IceUtil::Monitor<IceUtil::Mutex > > _monitor;
#endif

    enum
    {
	StateUnsent,
	StateInProgress,
	StateOK,
	StateUserException,
	StateLocalException
    } _state;

    BasicStream _is;
    BasicStream _os;
};

}

#endif
