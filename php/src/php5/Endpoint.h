//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPHP_ENDPOINT_H
#define ICEPHP_ENDPOINT_H

#include <Config.h>

namespace IcePHP
{

bool endpointInit(TSRMLS_D);

bool createEndpoint(zval*, const Ice::EndpointPtr& TSRMLS_DC);
bool fetchEndpoint(zval*, Ice::EndpointPtr& TSRMLS_DC);

bool createEndpointInfo(zval*, const Ice::EndpointInfoPtr& TSRMLS_DC);

} // End of namespace IcePHP

#endif
