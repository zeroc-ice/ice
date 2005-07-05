// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
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

namespace IceE
{

class LocalException;

}

namespace IceEInternal
{

//
// An exception wrapper, which is used to indicate that an operation
// that failed due to an exception is not repeatable if "at-most-once"
// semantics must be guaranteed.
//
class ICEE_API NonRepeatable
{
public:

    NonRepeatable(const NonRepeatable&);
    NonRepeatable(const IceE::LocalException&);
    const IceE::LocalException* get() const;

private:

    const NonRepeatable& operator=(const NonRepeatable&);

    std::auto_ptr<IceE::LocalException> _ex;
};

class ICEE_API Outgoing : private IceE::noncopyable, public IceE::Monitor<IceE::Mutex >
{
public:

    Outgoing(IceE::Connection*, Reference*, const std::string&, IceE::OperationMode, const IceE::Context&);

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const IceE::LocalException&);
    void finished(BasicStream&);
    void finished(const IceE::LocalException&);

    // Inlined for speed optimization.
    BasicStream* is() { return &_is; }
    BasicStream* os() { return &_os; }

private:

    //
    // Optimization. The connection and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    IceE::Connection* _connection;
    Reference* _reference;

    std::auto_ptr<IceE::LocalException> _exception;

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
