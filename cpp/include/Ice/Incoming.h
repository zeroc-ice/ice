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

#ifndef ICE_INCOMING_H
#define ICE_INCOMING_H

#include <Ice/InstanceF.h>
#include <Ice/ConnectionF.h>
#include <Ice/ServantLocatorF.h>
#include <Ice/IncomingAsyncF.h>
#include <Ice/BasicStream.h>
#include <Ice/Current.h>

namespace IceInternal
{

class ICE_API Incoming : public ::IceUtil::noncopyable
{
public:

    Incoming(const InstancePtr&, const ::Ice::ObjectAdapterPtr&, ::IceInternal::Connection*, bool, bool);

    void invoke();

    BasicStream* is();
    BasicStream* os();

private:
    
    void finishInvoke();
    void warning(const ::Ice::Exception&) const;
    void warning(const std::string&) const;

    //
    // IncomingAsync needs access to the various data members
    // below. Without making IncomingAsync a friend class, we would
    // have to write lots of otherwise useless accessors.
    //
    friend class IncomingAsync;

    Ice::Current _current;
    Ice::ObjectPtr _servant;
    Ice::ServantLocatorPtr _locator;
    Ice::LocalObjectPtr _cookie;

    //
    // Optimization. The connection may not be deleted while a
    // stack-allocated Incoming still holds it.
    //
    Connection* _connection;

    bool _response;
    bool _compress;

    BasicStream _is;
    BasicStream _os;
};

}

#endif
