// **********************************************************************
//
// Copyright (c) 2003-2009 ZeroC, Inc. All rights reserved.
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
ZEND_FUNCTION(Ice_stringVersion);
ZEND_FUNCTION(Ice_intVersion);
ZEND_FUNCTION(Ice_loadProfile);
ZEND_FUNCTION(Ice_loadProfileWithArgs);
ZEND_FUNCTION(Ice_dumpProfile);
}

#define ICE_PHP_PROFILE_FUNCTIONS \
    ZEND_FE(Ice_stringVersion,       NULL) \
    ZEND_FE(Ice_intVersion,          NULL) \
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
class Profile
{
public:

    typedef std::map<std::string, Slice::ClassDefPtr> ClassMap;

    Profile(const std::string&, const std::vector<Slice::UnitPtr>&, const std::string&, const ClassMap&,
            const Ice::PropertiesPtr&);

    std::string name() const;
    std::string code() const;
    const ClassMap& classes() const;
    Ice::PropertiesPtr properties() const;

    Slice::TypePtr lookupType(const std::string&) const;
    Slice::ExceptionPtr lookupException(const std::string&) const;

    void destroy(TSRMLS_D);

private:

    std::string _name;                  // The profile name
    std::vector<Slice::UnitPtr> _units; // The parsed Slice files
    std::string _code;                  // PHP code generated from Slice types
    ClassMap _classes;                  // Associates flattened type ids to their ClassDefs
    Ice::PropertiesPtr _properties;     // Configuration properties
};

} // End of namespace IcePHP

#endif
