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

#ifndef ICE_PHP_ICE_PROFILE_H
#define ICE_PHP_ICE_PROFILE_H

#include "ice_common.h"

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
