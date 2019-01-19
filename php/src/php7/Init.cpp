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

ZEND_BEGIN_ARG_INFO_EX(Ice_initialize_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(-1))
    ZEND_ARG_PASS_INFO(1)
    ZEND_ARG_PASS_INFO(1)
ZEND_END_ARG_INFO()

ZEND_BEGIN_ARG_INFO_EX(Ice_createProperties_arginfo, 1, ZEND_RETURN_VALUE, static_cast<zend_ulong>(-1))
    ZEND_ARG_PASS_INFO(1)
    ZEND_ARG_PASS_INFO(0)
ZEND_END_ARG_INFO()

#ifdef ICEPHP_USE_NAMESPACES
#  define ICEPHP_COMMUNICATOR_FUNCTIONS \
    ZEND_NS_NAMED_FE("Ice", initialize, ZEND_FN(Ice_initialize), Ice_initialize_arginfo) \
    ZEND_NS_NAMED_FE("Ice", register, ZEND_FN(Ice_register), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", unregister, ZEND_FN(Ice_unregister), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", find, ZEND_FN(Ice_find), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", getProperties, ZEND_FN(Ice_getProperties), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", identityToString, ZEND_FN(Ice_identityToString), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", stringToIdentity, ZEND_FN(Ice_stringToIdentity), ICE_NULLPTR)
#else
#  define ICEPHP_COMMUNICATOR_FUNCTIONS \
    ZEND_FE(Ice_initialize, Ice_initialize_arginfo) \
    ZEND_FE(Ice_register, ICE_NULLPTR) \
    ZEND_FE(Ice_unregister, ICE_NULLPTR) \
    ZEND_FE(Ice_find, ICE_NULLPTR) \
    ZEND_FE(Ice_getProperties, ICE_NULLPTR) \
    ZEND_FE(Ice_identityToString, ICE_NULLPTR) \
    ZEND_FE(Ice_stringToIdentity, ICE_NULLPTR)
#endif

#define ICEPHP_OPERATION_FUNCTIONS \
    ZEND_FE(IcePHP_defineOperation,  ICE_NULLPTR)

#ifdef ICEPHP_USE_NAMESPACES
#  define ICEPHP_PROPERTIES_FUNCTIONS \
    ZEND_NS_NAMED_FE("Ice", createProperties, ZEND_FN(Ice_createProperties), Ice_createProperties_arginfo)
#else
#  define ICEPHP_PROPERTIES_FUNCTIONS \
    ZEND_FE(Ice_createProperties, Ice_createProperties_arginfo)
#endif

#define ICEPHP_TYPE_FUNCTIONS \
    ZEND_FE(IcePHP_defineEnum,          ICE_NULLPTR) \
    ZEND_FE(IcePHP_defineStruct,        ICE_NULLPTR) \
    ZEND_FE(IcePHP_defineSequence,      ICE_NULLPTR) \
    ZEND_FE(IcePHP_defineDictionary,    ICE_NULLPTR) \
    ZEND_FE(IcePHP_declareProxy,        ICE_NULLPTR) \
    ZEND_FE(IcePHP_defineProxy,         ICE_NULLPTR) \
    ZEND_FE(IcePHP_declareClass,        ICE_NULLPTR) \
    ZEND_FE(IcePHP_defineClass,         ICE_NULLPTR) \
    ZEND_FE(IcePHP_defineException,     ICE_NULLPTR) \
    ZEND_FE(IcePHP_stringify,           ICE_NULLPTR) \
    ZEND_FE(IcePHP_stringifyException,  ICE_NULLPTR)

#ifdef ICEPHP_USE_NAMESPACES
#  define ICEPHP_UTIL_FUNCTIONS \
    ZEND_NS_NAMED_FE("Ice", stringVersion, ZEND_FN(Ice_stringVersion), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", intVersion, ZEND_FN(Ice_intVersion), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", generateUUID, ZEND_FN(Ice_generateUUID), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", currentProtocol, ZEND_FN(Ice_currentProtocol), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", currentProtocolEncoding, ZEND_FN(Ice_currentProtocolEncoding), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", currentEncoding, ZEND_FN(Ice_currentEncoding), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", protocolVersionToString, ZEND_FN(Ice_protocolVersionToString), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", stringToProtocolVersion, ZEND_FN(Ice_stringToProtocolVersion), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", encodingVersionToString, ZEND_FN(Ice_encodingVersionToString), ICE_NULLPTR) \
    ZEND_NS_NAMED_FE("Ice", stringToEncodingVersion, ZEND_FN(Ice_stringToEncodingVersion), ICE_NULLPTR)
#else
#  define ICEPHP_UTIL_FUNCTIONS \
    ZEND_FE(Ice_stringVersion, ICE_NULLPTR) \
    ZEND_FE(Ice_intVersion, ICE_NULLPTR) \
    ZEND_FE(Ice_generateUUID, ICE_NULLPTR) \
    ZEND_FE(Ice_currentProtocol, ICE_NULLPTR) \
    ZEND_FE(Ice_currentProtocolEncoding, ICE_NULLPTR) \
    ZEND_FE(Ice_currentEncoding, ICE_NULLPTR) \
    ZEND_FE(Ice_protocolVersionToString, ICE_NULLPTR) \
    ZEND_FE(Ice_stringToProtocolVersion, ICE_NULLPTR) \
    ZEND_FE(Ice_encodingVersionToString, ICE_NULLPTR) \
    ZEND_FE(Ice_stringToEncodingVersion, ICE_NULLPTR)
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

    int status = SUCCESS;

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
