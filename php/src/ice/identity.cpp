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

#include "ice_identity.h"
#include "ice_util.h"

using namespace std;

bool
Ice_Identity_create(zval* zv, const Ice::Identity& id TSRMLS_DC)
{
    zend_class_entry* cls = ice_findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    if(object_init_ex(zv, cls) != SUCCESS)
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

    zend_class_entry* cls = ice_findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    zend_class_entry* ce = Z_OBJCE_P(zv);
    if(ce != cls)
    {
        zend_error(E_ERROR, "expected an identity but received %s", ce->name);
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

    if(categoryVal && Z_TYPE_PP(categoryVal) != IS_STRING && Z_TYPE_PP(categoryVal) != IS_NULL)
    {
        string s = ice_zendTypeToString(Z_TYPE_PP(categoryVal));
        zend_error(E_ERROR, "expected a string value for identity member `category' but received %s", s.c_str());
        return false;
    }

    id.name = Z_STRVAL_PP(nameVal);
    if(categoryVal && Z_TYPE_PP(categoryVal) == IS_STRING)
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
        ice_throwException(ex TSRMLS_CC);
    }
}

ZEND_FUNCTION(Ice_identityToString)
{
    if(ZEND_NUM_ARGS() != 1)
    {
        WRONG_PARAM_COUNT;
    }

    zend_class_entry* cls = ice_findClass("Ice_Identity" TSRMLS_CC);
    assert(cls);

    zval *zid;

    if(zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "O", &zid, cls) == FAILURE)
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
