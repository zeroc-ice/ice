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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "identity.h"
#include "exception.h"
#include "util.h"

using namespace std;

ZEND_EXTERN_MODULE_GLOBALS(ice)

bool
Ice_Identity_create(zval* zv, const Ice::Identity& id TSRMLS_DC)
{
    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find("::Ice::Identity");
    assert(p != typeMap->end());

    if(object_init_ex(zv, p->second) != SUCCESS)
    {
        zend_error(E_ERROR, "unable to initialize Ice::Identity in %s()", get_active_function_name(TSRMLS_C));
        return false;
    }

    add_property_stringl_ex(zv, "name", sizeof("name"), const_cast<char*>(id.name.c_str()),
                            id.name.length(), 1 TSRMLS_CC);
    add_property_stringl_ex(zv, "category", sizeof("category"), const_cast<char*>(id.category.c_str()),
                            id.category.length(), 1 TSRMLS_CC);

    return true;
}

bool
Ice_Identity_extract(zval* zv, Ice::Identity& id TSRMLS_DC)
{
    if(Z_TYPE_P(zv) != IS_OBJECT)
    {
        zend_error(E_ERROR, "value does not contain an object");
        return false;
    }

    zend_object* obj = static_cast<zend_object*>(zend_object_store_get_object(zv TSRMLS_CC));
    if(!obj)
    {
        zend_error(E_ERROR, "object not found in object store");
        return false;
    }

    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find("::Ice::Identity");
    assert(p != typeMap->end());

    if(obj->ce != p->second)
    {
        zend_error(E_ERROR, "expected an identity but received %s", obj->ce->name);
        return false;
    }

    //
    // Category is optional, but name is required.
    //
    zval** categoryVal = NULL;
    zval** nameVal;
    if(zend_hash_find(Z_OBJPROP_P(zv), "name", sizeof("name"), (void**)&nameVal) == FAILURE)
    {
        zend_error(E_ERROR, "identity value does not contain member `name'");
        return false;
    }
    zend_hash_find(Z_OBJPROP_P(zv), "category", sizeof("category"), (void**)&categoryVal);

    if(Z_TYPE_PP(nameVal) != IS_STRING)
    {
        string s = ice_zendTypeToString(Z_TYPE_PP(nameVal));
        zend_error(E_ERROR, "expected a string value for identity member `name' but received %s", s.c_str());
        return false;
    }

    if(categoryVal && Z_TYPE_PP(categoryVal) != IS_STRING)
    {
        string s = ice_zendTypeToString(Z_TYPE_PP(categoryVal));
        zend_error(E_ERROR, "expected a string value for identity member `category' but received %s", s.c_str());
        return false;
    }

    id.name = Z_STRVAL_PP(nameVal);
    if(categoryVal)
    {
        id.category = Z_STRVAL_PP(categoryVal);
    }
    else
    {
        id.category = "";
    }

    return true;
}

ZEND_FUNCTION(Ice_stringToIdentity)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    char* str;
    int len;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &len) == FAILURE)
    {
        RETURN_NULL();
    }

    try
    {
        Ice::Identity id = Ice::stringToIdentity(str);
        Ice_Identity_create(return_value, id TSRMLS_CC);
    }
    catch(const IceUtil::Exception& ex)
    {
        ice_throw_exception(ex TSRMLS_CC);
    }
}

ZEND_FUNCTION(Ice_identityToString)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    TypeMap* typeMap = static_cast<TypeMap*>(ICE_G(typeMap));
    TypeMap::iterator p = typeMap->find("::Ice::Identity");
    assert(p != typeMap->end());

    zval *zid;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zid, p->second) == FAILURE)
    {
        RETURN_NULL();
    }

    Ice::Identity id;
    if(Ice_Identity_extract(zid, id TSRMLS_CC))
    {
        string s = Ice::identityToString(id);
        RETURN_STRINGL(const_cast<char*>(s.c_str()), s.length(), 1);
    }
}
