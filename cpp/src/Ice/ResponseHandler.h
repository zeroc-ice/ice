//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_RESPONSE_HANDLER_H
#define ICE_RESPONSE_HANDLER_H

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

class ResponseHandler : public EnableSharedFromThis<ResponseHandler>
{
public:

    virtual void sendResponse(Ice::Int, Ice::OutputStream*, Ice::Byte, bool) = 0;
    virtual void sendNoResponse() = 0;
    virtual bool systemException(Ice::Int, std::exception_ptr, bool) = 0;
    virtual void invokeException(Ice::Int, std::exception_ptr, int, bool) = 0;
};

}

#endif
