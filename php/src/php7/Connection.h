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

bool connectionInit(void);

bool createConnection(zval*, const Ice::ConnectionPtr&);
bool fetchConnection(zval*, Ice::ConnectionPtr&);

bool createConnectionInfo(zval*, const Ice::ConnectionInfoPtr&);

} // End of namespace IcePHP

#endif
