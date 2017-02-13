// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_REQUEST_HANDLER_F_H
#define ICE_REQUEST_HANDLER_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class CancellationHandler;
class RequestHandler;

#ifdef ICE_CPP11_MAPPING
using CancellationHandlerPtr = ::std::shared_ptr<CancellationHandler>;
using RequestHandlerPtr = ::std::shared_ptr<RequestHandler>;
#else
ICE_API IceUtil::Shared* upCast(CancellationHandler*);
typedef IceInternal::Handle<CancellationHandler> CancellationHandlerPtr;

ICE_API IceUtil::Shared* upCast(RequestHandler*);
typedef IceInternal::Handle<RequestHandler> RequestHandlerPtr;
#endif

}

#endif
