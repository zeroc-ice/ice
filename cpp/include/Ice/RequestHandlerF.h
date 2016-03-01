// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

class RequestHandler;
ICE_API IceUtil::Shared* upCast(RequestHandler*);
typedef IceInternal::Handle<RequestHandler> RequestHandlerPtr;

class CancellationHandler;
ICE_API IceUtil::Shared* upCast(CancellationHandler*);
typedef IceInternal::Handle<CancellationHandler> CancellationHandlerPtr;

}

#endif
