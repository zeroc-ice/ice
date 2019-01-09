// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
