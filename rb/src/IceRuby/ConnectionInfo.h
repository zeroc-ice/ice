// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_CONNECTION_INFO_H
#define ICE_RUBY_CONNECTION_INFO_H

#include <Config.h>
#include <Ice/Connection.h>

namespace IceRuby
{

void initConnectionInfo(VALUE);
VALUE createConnectionInfo(const Ice::ConnectionInfoPtr&);

VALUE createEndpointInfo(const Ice::EndpointInfoPtr&);

}

#endif
