// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Endpoint.h>

namespace IceMatlab
{

void* createEndpoint(std::shared_ptr<Ice::Endpoint>);
std::shared_ptr<Ice::Endpoint> getEndpoint(void*);

}
