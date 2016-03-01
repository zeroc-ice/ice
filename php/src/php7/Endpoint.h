// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_ENDPOINT_H
#define ICEPHP_ENDPOINT_H

#include <Config.h>

namespace IcePHP
{

bool endpointInit(void);

bool createEndpoint(zval*, const Ice::EndpointPtr&);
bool fetchEndpoint(zval*, Ice::EndpointPtr&);

bool createEndpointInfo(zval*, const Ice::EndpointInfoPtr&);

} // End of namespace IcePHP

#endif
