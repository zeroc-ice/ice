// **********************************************************************
//
// Copyright (c) 2001
// MutableRealms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#ifndef ICE_OUTGOING_H
#define ICE_OUTGOING_H

#include <Ice/EmitterF.h>
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

class ICE_API Outgoing : public ::IceUtil::noncopyable, public JTCMonitorT< JTCMutex >
{
public:

    Outgoing(const EmitterPtr&, const ReferencePtr&, bool, const char*, const ::Ice::Context&);
    ~Outgoing();

    bool invoke();
    void finished(BasicStream&);
    void finished(const ::Ice::LocalException&);

    BasicStream* is();
    BasicStream* os();

private:

    EmitterPtr _emitter;
    ReferencePtr _reference;
    bool _sendRef;
    std::auto_ptr< ::Ice::LocalException> _exception;

    enum
    {
	StateUnsent,
	StateInProgress,
	StateOK,
	StateException,
	StateLocationForward,
	StateLocalException,
    } _state;

    BasicStream _is;
    BasicStream _os;
};

}

#endif
