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

    virtual void sendResponse(std::int32_t, Ice::OutputStream*, std::uint8_t, bool) = 0;
    virtual void sendNoResponse() = 0;
    virtual bool systemException(std::int32_t, std::exception_ptr, bool) = 0;
    virtual void invokeException(std::int32_t, std::exception_ptr, int, bool) = 0;
};

}

#endif
