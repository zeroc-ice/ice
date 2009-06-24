// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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

bool endpointInit(TSRMLS_D);

bool createEndpoint(zval*, const Ice::EndpointPtr& TSRMLS_DC);
bool fetchEndpoint(zval*, Ice::EndpointPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* endpointClassEntry;

} // End of namespace IcePHP

#endif
