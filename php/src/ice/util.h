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

#ifndef ICE_PHP_UTIL_H
#define ICE_PHP_UTIL_H

extern "C"
{
#include "php.h"
#include "php_ini.h"
#include "ext/standard/info.h"
}

#include <Ice/Ice.h>

//
// PHP wrapper for C++ objects.
//
struct ice_object
{
    zend_object zobj;
    void* ptr; // For object data.
};

//
// Create a new ice_object for a class entry. The allocator registered for the
// class entry will be invoked, but the C++ object is not created here.
//
ice_object* ice_object_new(zend_class_entry* TSRMLS_DC);

//
// Retrieve the ice_object given a zval.
//
ice_object* ice_object_get(zval* TSRMLS_DC);

//
// Split a string into a vector of arguments. Quoted arguments are supported.
//
bool ice_split_string(const std::string&, std::vector<std::string>&);

//
// Convert a string to lowercase.
//
std::string ice_lowercase(const std::string&);

//
// Flatten a scoped name.
//
std::string ice_flatten(const std::string&);

#endif
