// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_INCOMING_REQUEST_H
#define ICE_INCOMING_REQUEST_H

#include <Ice/Incoming.h>
#include <Ice/Object.h>

namespace IceInternal
{

//
// Adapts Incoming to Ice::Request
// (the goal here is to avoid adding any virtual function to Incoming)
//
class ICE_API IncomingRequest : public Ice::Request
{
public:
    IncomingRequest(Incoming& in) :
        _in(in)
    {
    }

    virtual const Ice::Current& getCurrent();
    
    Incoming& _in;
};
}

#endif
