// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_ENDPOINT_H
#define ICE_RUBY_ENDPOINT_H

#include <Config.h>
#include <Ice/Connection.h>

namespace IceRuby
{

void initEndpoint(VALUE);

VALUE createEndpoint(const Ice::EndpointPtr&);
VALUE createEndpointInfo(const Ice::EndpointInfoPtr&);
bool checkEndpoint(VALUE);

}

#endif
