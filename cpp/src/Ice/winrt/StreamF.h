// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_WINRT_STREAM_F_H
#define ICE_WINRT_STREAM_F_H

#include <IceUtil/Shared.h>
#include <Ice/Handle.h>

namespace IceInternal
{

class StreamEndpointI;
ICE_API IceUtil::Shared* upCast(StreamEndpointI*);
typedef IceInternal::Handle<StreamEndpointI> StreamEndpointIPtr;

class StreamAcceptor;
ICE_API IceUtil::Shared* upCast(StreamAcceptor*);
typedef IceInternal::Handle<StreamAcceptor> StreamAcceptorPtr;

}

#endif
