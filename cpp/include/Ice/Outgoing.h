// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Monitor.h>
#include <Ice/ConnectionIF.h>
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
    NonRepeatable(const Ice::LocalException&);
    const Ice::LocalException* get() const;

private:

    const NonRepeatable& operator=(const NonRepeatable&);

    std::auto_ptr<Ice::LocalException> _ex;
};

class ICE_API Outgoing : public IceUtil::noncopyable, public IceUtil::Monitor<IceUtil::Mutex >
{
public:

    Outgoing(Ice::ConnectionI*, Reference*, const std::string&, Ice::OperationMode, const Ice::Context&, bool);

    bool invoke(); // Returns true if ok, false if user exception.
    void abort(const Ice::LocalException&);
    void finished(BasicStream&);
    void finished(const Ice::LocalException&);

    // Inlined for speed optimization.
    BasicStream* is() { return &_is; }
    BasicStream* os() { return &_os; }

private:

    //
    // Optimization. The connection and the reference may not be
    // deleted while a stack-allocated Outgoing still holds it.
    //
    Ice::ConnectionI* _connection;
    Reference* _reference;

    std::auto_ptr<Ice::LocalException> _exception;

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

    const bool _compress;
};

}

#endif
