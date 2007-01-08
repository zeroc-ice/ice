// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_PHP_PROFILE_H
#define ICE_PHP_PROFILE_H

#include <Config.h>

//
// Global functions.
//
extern "C"
{
ZEND_FUNCTION(Ice_loadProfile);
ZEND_FUNCTION(Ice_loadProfileWithArgs);
ZEND_FUNCTION(Ice_dumpProfile);
}

#define ICE_PHP_PROFILE_FUNCTIONS \
    ZEND_FE(Ice_loadProfile,         NULL) \
    ZEND_FE(Ice_loadProfileWithArgs, NULL) \
    ZEND_FE(Ice_dumpProfile,         NULL)

namespace IcePHP
{

bool profileInit(TSRMLS_D);
bool profileShutdown(TSRMLS_D);

//
// Profile contains information about an Ice configuration. A pointer to a Profile instance
// is stored in the PHP globals (see php_ice.h) when a script invokes Ice_loadProfile().
//
struct Profile
{
    typedef std::map<std::string, Slice::ClassDefPtr> ClassMap;

    std::string name;              // The profile name
    Slice::UnitPtr unit;           // The parsed Slice code
    std::string code;              // PHP code generated from Slice types
    ClassMap classes;              // Associates flattened type ids to their ClassDefs
    Ice::PropertiesPtr properties; // Configuration properties
};

} // End of namespace IcePHP

#endif
