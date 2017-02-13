// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
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
#include <Ice/VirtualShared.h>

namespace Ice
{

class OutputStream;

}

namespace IceInternal
{

class ResponseHandler :
#ifdef ICE_CPP11_MAPPING
        public EnableSharedFromThis<ResponseHandler>
#else
        public virtual IceUtil::Shared
#endif
{
public:

    virtual void sendResponse(Ice::Int, Ice::OutputStream*, Ice::Byte, bool) = 0;
    virtual void sendNoResponse() = 0;
    virtual bool systemException(Ice::Int, const Ice::SystemException&, bool) = 0;
    virtual void invokeException(Ice::Int, const Ice::LocalException&, int, bool) = 0;
};

}

#endif
