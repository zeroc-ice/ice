// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PHP_UTIL_H
#define ICE_PHP_UTIL_H

#include <Config.h>

//
// Ice_Identity global functions.
//
extern "C"
{
ZEND_FUNCTION(Ice_stringToIdentity);
ZEND_FUNCTION(Ice_identityToString);
}

#define ICE_PHP_IDENTITY_FUNCTIONS \
    ZEND_FE(Ice_stringToIdentity,   NULL) \
    ZEND_FE(Ice_identityToString,   NULL)

namespace IcePHP
{

bool createIdentity(zval*, const Ice::Identity& TSRMLS_DC);
bool extractIdentity(zval*, Ice::Identity& TSRMLS_DC);

bool createContext(zval*, const Ice::Context& TSRMLS_DC);
bool extractContext(zval*, Ice::Context& TSRMLS_DC);

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
ice_object* newObject(zend_class_entry* TSRMLS_DC);

//
// Retrieve the ice_object given a zval.
//
ice_object* getObject(zval* TSRMLS_DC);

//
// Convert the given exception into a PHP equivalent and "throw" it.
//
void throwException(const IceUtil::Exception& TSRMLS_DC);

//
// Find the class entry for a flattened type name.
//
zend_class_entry* findClass(const std::string& TSRMLS_DC);

//
// Find the class entry for a scoped type with suffix.
//
zend_class_entry* findClassScoped(const std::string& TSRMLS_DC);

//
// Convert a string to lowercase.
//
std::string lowerCase(const std::string&);

//
// Flatten a scoped name. Leading "::" is removed, and all remaining "::"
// are replaced with underscores. The resulting string is then escaped if it
// conflicts with a PHP keyword.
//
std::string flatten(const std::string&);

//
// Check the given identifier against PHP's list of reserved words. If it matches
// a reserved word, then an escaped version is returned with a leading underscore.
//
std::string fixIdent(const std::string&);

//
// Convert a Zend type (e.g., IS_BOOL, etc.) to a string for use in error messages.
//
std::string zendTypeToString(int);

//
// Returns true if the given type is valid for use as a key in a native PHP associative array.
//
bool isNativeKey(const Slice::TypePtr&);

//
// Determines whether a class (or interface) inherits from a base class (or interface).
//
bool checkClass(zend_class_entry*, zend_class_entry*);

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

//
// Exception-safe zval destroy.
//
class AutoDestroy
{
public:
    AutoDestroy(zval* zv) : _zv(zv) {}
    ~AutoDestroy() { if(_zv) zval_ptr_dtor(&_zv); }

private:
    zval* _zv;
};

} // End of namespace IcePHP

#endif
