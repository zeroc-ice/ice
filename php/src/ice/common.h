// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#ifndef ICE_PHP_COMMON_H
#define ICE_PHP_COMMON_H

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include "php_ice.h"
#include <Ice/Ice.h>

//
// Extension of zend_class_entry that stores some extra data.
// This only works for user class entries, not internal class
// entries, because Zend makes a private copy of internal class
// entries.
//
struct ice_class_entry
{
    zend_class_entry ce;
    char* scoped;
    void* syntaxTreeBase;
    void* marshaler;
};

//
// Map from scoped name to zend_class_entry*.
//
typedef std::map<std::string, zend_class_entry*> TypeMap;

#endif
