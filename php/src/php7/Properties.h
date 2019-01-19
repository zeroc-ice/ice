//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPHP_PROPERTIES_H
#define ICEPHP_PROPERTIES_H

#include <Config.h>

//
// Global functions.
//
extern "C"
{
ZEND_FUNCTION(Ice_createProperties);
}

namespace IcePHP
{

bool propertiesInit(void);

bool createProperties(zval*, const Ice::PropertiesPtr&);
bool fetchProperties(zval*, Ice::PropertiesPtr&);

//
// Class entry.
//
extern zend_class_entry* propertiesClassEntry;

} // End of namespace IcePHP

#endif
