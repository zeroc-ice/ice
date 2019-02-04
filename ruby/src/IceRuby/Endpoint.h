//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
