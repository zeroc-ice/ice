//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Communicator.h>
#include <Connection.h>
#include <Endpoint.h>
#include <Logger.h>
#include <Operation.h>
#include <Properties.h>
#include <Proxy.h>
#include <Types.h>
#include <Util.h>

using namespace std;
using namespace IcePHP;

ZEND_DECLARE_MODULE_GLOBALS(ice)

ZEND_BEGIN_ARG_INFO_EX(Ice_initialize_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(0))
    ZEND_ARG_INFO(1, args)
    ZEND_ARG_INFO(1, properties)
    ZEND_ARG_INFO(1, initData)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_createProperties_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(0))
    ZEND_ARG_INFO(0, args)
    ZEND_ARG_INFO(0, defaults)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_declareClass_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineClass_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(7))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, compactId)
    ZEND_ARG_INFO(0, preserve)
    ZEND_ARG_INFO(0, interface)
    ZEND_ARG_INFO(0, base)
    ZEND_ARG_INFO(0, members)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineDictionary_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(3))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, key)
    ZEND_ARG_INFO(0, value)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineEnum_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, enumerators)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineException_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(5))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, preserve)
    ZEND_ARG_INFO(0, base)
    ZEND_ARG_INFO(0, members)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineStruct_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(3))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, members)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_declareProxy_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineProxy_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(3))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, compactId)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineSequence_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, element)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_defineOperation_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(9))
    ZEND_ARG_INFO(0, cls)
    ZEND_ARG_INFO(0, name)
    ZEND_ARG_INFO(0, mode)
    ZEND_ARG_INFO(0, sendMode)
    ZEND_ARG_INFO(0, format)
    ZEND_ARG_INFO(0, inParams)
    ZEND_ARG_INFO(0, outParams)
    ZEND_ARG_INFO(0, returnType)
    ZEND_ARG_INFO(0, exceptions)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_register_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, communicator)
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, expires)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_unregister_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_find_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_getProperties_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(0))
    ZEND_ARG_INFO(0, name)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_identityToString_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, id)
    ZEND_ARG_INFO(0, mode)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_stringToIdentity_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, id)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_stringify_add_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_stringifyException_add_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(2))
    ZEND_ARG_INFO(0, value)
    ZEND_ARG_INFO(0, type)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_protocolVersionToString_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, protocolVersion)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_stringToProtocolVersion_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_encodingVersionToString_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, encodingVersion)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_stringToEncodingVersion_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(1))
    ZEND_ARG_INFO(0, str)
ZEND_END_ARG_INFO()

#ifdef ICEPHP_USE_NAMESPACES
#  define ICEPHP_COMMUNICATOR_FUNCTIONS \
    ZEND_NS_NAMED_FE("Ice", initialize, ZEND_FN(Ice_initialize), Ice_initialize_arginfo) \
    ZEND_NS_NAMED_FE("Ice", register, ZEND_FN(Ice_register), Ice_register_arginfo) \
    ZEND_NS_NAMED_FE("Ice", unregister, ZEND_FN(Ice_unregister), Ice_unregister_arginfo) \
    ZEND_NS_NAMED_FE("Ice", find, ZEND_FN(Ice_find), Ice_find_arginfo) \
    ZEND_NS_NAMED_FE("Ice", getProperties, ZEND_FN(Ice_getProperties), Ice_getProperties_arginfo) \
    ZEND_NS_NAMED_FE("Ice", identityToString, ZEND_FN(Ice_identityToString), Ice_identityToString_arginfo) \
    ZEND_NS_NAMED_FE("Ice", stringToIdentity, ZEND_FN(Ice_stringToIdentity), Ice_stringToIdentity_arginfo)
#else
#  define ICEPHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_initialize, Ice_initialize_arginfo) \
    ZEND_FE(Ice_register, Ice_register_arginfo) \
    ZEND_FE(Ice_unregister, Ice_unregister_arginfo) \
    ZEND_FE(Ice_find, Ice_find_arginfo) \
    ZEND_FE(Ice_getProperties, Ice_getProperties_arginfo) \
    ZEND_FE(Ice_identityToString, Ice_identityToString_arginfo) \
    ZEND_FE(Ice_stringToIdentity, Ice_stringToIdentity_arginfo)
#endif

#define ICEPHP_OPERATION_FUNCTIONS \
    ZEND_FE(IcePHP_defineOperation,  Ice_defineOperation_arginfo)

#ifdef ICEPHP_USE_NAMESPACES
#  define ICEPHP_PROPERTIES_FUNCTIONS \
    ZEND_NS_NAMED_FE("Ice", createProperties, ZEND_FN(Ice_createProperties), Ice_createProperties_arginfo)
#else
#  define ICEPHP_PROPERTIES_FUNCTIONS \
    ZEND_FE(Ice_createProperties, Ice_createProperties_arginfo)
#endif

#define ICEPHP_TYPE_FUNCTIONS \
    ZEND_FE(IcePHP_defineEnum,          Ice_defineEnum_arginfo) \
    ZEND_FE(IcePHP_defineStruct,        Ice_defineStruct_arginfo) \
    ZEND_FE(IcePHP_defineSequence,      Ice_defineSequence_arginfo) \
    ZEND_FE(IcePHP_defineDictionary,    Ice_defineDictionary_arginfo) \
    ZEND_FE(IcePHP_declareProxy,        Ice_declareProxy_arginfo) \
    ZEND_FE(IcePHP_defineProxy,         Ice_defineProxy_arginfo) \
    ZEND_FE(IcePHP_declareClass,        Ice_declareClass_arginfo) \
    ZEND_FE(IcePHP_defineClass,         Ice_defineClass_arginfo) \
    ZEND_FE(IcePHP_defineException,     Ice_defineException_arginfo) \
    ZEND_FE(IcePHP_stringify,           Ice_stringify_add_arginfo) \
    ZEND_FE(IcePHP_stringifyException,  Ice_stringifyException_add_arginfo)

#ifdef ICEPHP_USE_NAMESPACES
#  define ICEPHP_UTIL_FUNCTIONS \
    ZEND_NS_NAMED_FE("Ice", stringVersion, ZEND_FN(Ice_stringVersion), ice_void_arginfo) \
    ZEND_NS_NAMED_FE("Ice", intVersion, ZEND_FN(Ice_intVersion), ice_void_arginfo) \
    ZEND_NS_NAMED_FE("Ice", generateUUID, ZEND_FN(Ice_generateUUID), ice_void_arginfo) \
    ZEND_NS_NAMED_FE("Ice", currentProtocol, ZEND_FN(Ice_currentProtocol), ice_void_arginfo) \
    ZEND_NS_NAMED_FE("Ice", currentProtocolEncoding, ZEND_FN(Ice_currentProtocolEncoding), ice_void_arginfo) \
    ZEND_NS_NAMED_FE("Ice", currentEncoding, ZEND_FN(Ice_currentEncoding), ice_void_arginfo) \
    ZEND_NS_NAMED_FE("Ice", protocolVersionToString, ZEND_FN(Ice_protocolVersionToString), Ice_protocolVersionToString_arginfo) \
    ZEND_NS_NAMED_FE("Ice", stringToProtocolVersion, ZEND_FN(Ice_stringToProtocolVersion), Ice_stringToProtocolVersion_arginfo) \
    ZEND_NS_NAMED_FE("Ice", encodingVersionToString, ZEND_FN(Ice_encodingVersionToString), Ice_encodingVersionToString_arginfo) \
    ZEND_NS_NAMED_FE("Ice", stringToEncodingVersion, ZEND_FN(Ice_stringToEncodingVersion), Ice_stringToEncodingVersion_arginfo)
#else
#  define ICEPHP_UTIL_FUNCTIONS \
    ZEND_FE(Ice_stringVersion, ice_void_arginfo) \
    ZEND_FE(Ice_intVersion, ice_void_arginfo) \
    ZEND_FE(Ice_generateUUID, ice_void_arginfo) \
    ZEND_FE(Ice_currentProtocol, ice_void_arginfo) \
    ZEND_FE(Ice_currentProtocolEncoding, ice_void_arginfo) \
    ZEND_FE(Ice_currentEncoding, ice_void_arginfo) \
    ZEND_FE(Ice_protocolVersionToString, Ice_protocolVersionToString_arginfo) \
    ZEND_FE(Ice_stringToProtocolVersion, Ice_stringToProtocolVersion_arginfo) \
    ZEND_FE(Ice_encodingVersionToString, Ice_encodingVersionToString_arginfo) \
    ZEND_FE(Ice_stringToEncodingVersion, Ice_stringToEncodingVersion_arginfo)
#endif

//
// Necessary to suppress warnings from zend_function_entry in php-5.2.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

//
// Entries for all global functions.
//
zend_function_entry ice_functions[] =
{
    ICEPHP_COMMUNICATOR_FUNCTIONS
    ICEPHP_OPERATION_FUNCTIONS
    ICEPHP_PROPERTIES_FUNCTIONS
    ICEPHP_TYPE_FUNCTIONS
    ICEPHP_UTIL_FUNCTIONS
    {0, 0, 0}
};
//
// enable warning again
//
#if defined(__GNUC__)
#  pragma GCC diagnostic error "-Wwrite-strings"
#endif

//
// The ice_module_entry declaration below generates lots of warnings:
//
// deprecated conversion from string constant to ‘char*’
//
// We disable them with a pragma.
//
#if defined(__GNUC__)
#  pragma GCC diagnostic ignored "-Wwrite-strings"
#endif

zend_module_entry ice_module_entry =
{
    STANDARD_MODULE_HEADER,
    "ice",
    ice_functions,
    ZEND_MINIT(ice),
    ZEND_MSHUTDOWN(ice),
    ZEND_RINIT(ice),
    ZEND_RSHUTDOWN(ice),
    ZEND_MINFO(ice),
    NO_VERSION_YET,
    STANDARD_MODULE_PROPERTIES
};

ZEND_GET_MODULE(ice)

//
// Declare initialization file entries.
//
PHP_INI_BEGIN()
  PHP_INI_ENTRY("ice.config", "", PHP_INI_SYSTEM, 0)
  PHP_INI_ENTRY("ice.options", "", PHP_INI_SYSTEM, 0)
  PHP_INI_ENTRY("ice.profiles", "", PHP_INI_SYSTEM, 0)
  PHP_INI_ENTRY("ice.hide_profiles", "1", PHP_INI_SYSTEM, 0)
PHP_INI_END()

extern "C"
int initIceGlobals(zend_ice_globals* g)
{
    g->communicatorMap = 0;
    g->idToClassInfoMap = 0;
    g->compactIdToClassInfoMap = 0;
    g->nameToClassInfoMap = 0;
    g->proxyInfoMap = 0;
    g->exceptionInfoMap = 0;
    g->unset = 0;
    return SUCCESS;
}

ZEND_MINIT_FUNCTION(ice)
{
    Ice::registerIceSSL(false);
    Ice::registerIceDiscovery(false);
    Ice::registerIceLocatorDiscovery(false);

    REGISTER_INI_ENTRIES();
    ZEND_INIT_MODULE_GLOBALS(ice, initIceGlobals, 0);

    if(!communicatorInit())
    {
        return FAILURE;
    }

    if(!propertiesInit())
    {
        return FAILURE;
    }

    if(!proxyInit())
    {
        return FAILURE;
    }

    if(!typesInit(INIT_FUNC_ARGS_PASSTHRU))
    {
        return FAILURE;
    }

    if(!loggerInit())
    {
        return FAILURE;
    }

    if(!endpointInit())
    {
        return FAILURE;
    }

    if(!connectionInit())
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_MSHUTDOWN_FUNCTION(ice)
{
    UNREGISTER_INI_ENTRIES();

    ZEND_RESULT_CODE status = SUCCESS;

    if(!communicatorShutdown())
    {
        status = FAILURE;
    }

    return status;
}

ZEND_RINIT_FUNCTION(ice)
{
    ICE_G(communicatorMap) = 0;

    if(!communicatorRequestInit())
    {
        return FAILURE;
    }

    if(!typesRequestInit())
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_RSHUTDOWN_FUNCTION(ice)
{
    if(!communicatorRequestShutdown())
    {
        return FAILURE;
    }

    if(!typesRequestShutdown())
    {
        return FAILURE;
    }

    return SUCCESS;
}

ZEND_MINFO_FUNCTION(ice)
{
    php_info_print_table_start();
    php_info_print_table_header(2, "Ice support", "enabled");
    php_info_print_table_row(2, "Ice version", ICE_STRING_VERSION);
    php_info_print_table_end();

    DISPLAY_INI_ENTRIES();
}
