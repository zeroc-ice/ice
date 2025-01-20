// Copyright (c) ZeroC, Inc.

#ifndef ICE_PHP_CONFIG_H
#define ICE_PHP_CONFIG_H

#include "Ice/Ice.h"

// Surpress various warnings emitted from including the PHP headers
#if defined(__clang__)
#    pragma clang diagnostic ignored "-Wconversion"
#    pragma clang diagnostic ignored "-Wsign-conversion"
#    pragma clang diagnostic ignored "-Wdocumentation"
#    pragma clang diagnostic ignored "-Wshadow"
#elif defined(__GNUC__)
#    pragma GCC diagnostic warning "-Wsign-compare"
#    pragma GCC diagnostic warning "-Wnarrowing"
#    pragma GCC diagnostic ignored "-Wignored-qualifiers"
#    pragma GCC diagnostic ignored "-Wredundant-decls"
#endif

// The php.h header defines/undefines NDEBUG based on how the PHP binary was built. As a result, asserts are always
// disabled unless building against a php binary built with --enable-debug. We want to enable asserts for the PHP Ice
// extension when it's built without OPTIMIZE=yes. We save NDEBUG in a tmp macro here and explicitly re-include the
// assert.h header with the saved NDEBUG macro after including php.h
#ifndef NDEBUG
#    define TMPDEBUG
#endif

// clang-format off
#include <php.h>
#include <php_ini.h>
#include <ext/standard/info.h>
#include <zend_exceptions.h>
#include <zend_interfaces.h>
// clang-format on

// Enable asserts if the extension is built with debug. It's fine to include several times assert.h with a different
// NDEBUG setting.
#ifdef TMPDEBUG
#    undef TMPDEBUG
#    undef NDEBUG
#    include <assert.h>
#endif

extern zend_module_entry ice_module_entry;
#define phpext_ice_ptr &ice_module_entry

#ifdef ZTS
//  If building for thread-safe enviroment, include the Thread Safe Resource Manager.
#    include "TSRM.h"
#endif

ZEND_MINIT_FUNCTION(ice);
ZEND_MSHUTDOWN_FUNCTION(ice);
ZEND_RINIT_FUNCTION(ice);
ZEND_RSHUTDOWN_FUNCTION(ice);
ZEND_MINFO_FUNCTION(ice);

ZEND_BEGIN_MODULE_GLOBALS(ice)
void* communicatorMap;
void* idToClassInfoMap;
void* compactIdToClassInfoMap;
void* nameToClassInfoMap;
void* proxyInfoMap;
void* exceptionInfoMap;
zval* unset;
ZEND_END_MODULE_GLOBALS(ice)

// A void arginfo used for methods which do not take parameters.
ZEND_BEGIN_ARG_INFO(ice_void_arginfo, 0)
ZEND_END_ARG_INFO()

// An arginfo used for __toString() methods.
#if PHP_VERSION_ID >= 80200
ZEND_BEGIN_ARG_WITH_RETURN_TYPE_INFO(ice_to_string_arginfo, IS_STRING, 0)
ZEND_END_ARG_INFO()
#else
// The return type declaration is optional with PHP < 8.2
ZEND_BEGIN_ARG_INFO(ice_to_string_arginfo, 0)
ZEND_END_ARG_INFO()
#endif

#ifdef ZTS
#    define ICE_G(v) TSRMG(ice_globals_id, zend_ice_globals*, v)
#else
#    define ICE_G(v) (ice_globals.v)
#endif

#ifndef Z_ADDREF_P
#    ifndef ZVAL_ADDREF
#        error "Unknown PHP version"
#    endif
#    define Z_ADDREF_P(zv) ZVAL_ADDREF(zv)
#endif

#ifndef ZEND_MN
#    define ZEND_MN(name) ZEND_FN(name)
#endif

#endif
