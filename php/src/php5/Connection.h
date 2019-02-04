//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPHP_CONNECTION_H
#define ICEPHP_CONNECTION_H

#include <Config.h>

namespace IcePHP
{

//
// Class entry.
//
extern zend_class_entry* connectionClassEntry;

bool connectionInit(TSRMLS_D);

bool createConnection(zval*, const Ice::ConnectionPtr& TSRMLS_DC);
bool fetchConnection(zval*, Ice::ConnectionPtr& TSRMLS_DC);

bool createConnectionInfo(zval*, const Ice::ConnectionInfoPtr& TSRMLS_DC);

} // End of namespace IcePHP

#endif
