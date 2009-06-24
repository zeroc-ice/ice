// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_CONNECTION_H
#define ICEPHP_CONNECTION_H

#include <Config.h>

namespace IcePHP
{

bool connectionInit(TSRMLS_D);

bool createConnection(zval*, const Ice::ConnectionPtr& TSRMLS_DC);
bool fetchConnection(zval*, Ice::ConnectionPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* connectionClassEntry;

} // End of namespace IcePHP

#endif
