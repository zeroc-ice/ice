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

#include "common.h"

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
ice_object* ice_newObject(zend_class_entry* TSRMLS_DC);

//
// Retrieve the ice_object given a zval.
//
ice_object* ice_getObject(zval* TSRMLS_DC);

//
// Split a string into a vector of arguments. Quoted arguments are supported.
//
bool ice_splitString(const std::string&, std::vector<std::string>&);

//
// Convert a string to lowercase.
//
std::string ice_lowerCase(const std::string&);

//
// Flatten a scoped name. Leading "::" is removed, and all remaining "::"
// are replaced with underscores. The resulting string is then escaped if it
// conflicts with a PHP keyword.
//
std::string ice_flatten(const std::string&);

//
// Check the given identifier against PHP's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
std::string ice_fixIdent(const std::string&);

//
// Convert a Zend type (e.g., IS_BOOL, etc.) to a string for use in error messages.
//
std::string ice_zendTypeToString(int);

//
// Exception-safe efree.
//
class AutoEfree
{
public:
    AutoEfree(void* p) : _p(p) {}
    ~AutoEfree() { efree(_p); }

private:
    void* _p;
};

#endif
