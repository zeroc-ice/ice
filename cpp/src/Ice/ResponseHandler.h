// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RESPONSE_HANDLER_H
#define ICE_RESPONSE_HANDLER_H

#include <IceUtil/Shared.h>
#include <IceUtil/Handle.h>

#include <Ice/Config.h>
#include <Ice/LocalException.h>
#include <Ice/ResponseHandlerF.h>

namespace IceInternal
{

class BasicStream;

class ResponseHandler : virtual public ::IceUtil::Shared
{
public:

    virtual ~ResponseHandler();

    virtual void sendResponse(Ice::Int, BasicStream*, Ice::Byte, bool) = 0;
    virtual void sendNoResponse() = 0;
    virtual bool systemException(Ice::Int, const Ice::SystemException&, bool) = 0;
    virtual void invokeException(Ice::Int, const Ice::LocalException&, int, bool) = 0;
};

}

#endif
