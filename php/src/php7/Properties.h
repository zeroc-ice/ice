// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
