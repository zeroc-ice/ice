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

#include "util.h"
#include <algorithm>
#include <ctype.h>

using namespace std;

ice_object*
ice_object_new(zend_class_entry* ce TSRMLS_DC)
{
    ice_object* obj;
    zval* tmp;

    obj = (ice_object*)emalloc(sizeof(ice_object));
    obj->zobj.ce = ce;
    obj->zobj.in_get = 0;
    obj->zobj.in_set = 0;
    obj->ptr = 0;

    ALLOC_HASHTABLE(obj->zobj.properties);
    zend_hash_init(obj->zobj.properties, 0, NULL, ZVAL_PTR_DTOR, 0);
    zend_hash_copy(obj->zobj.properties, &ce->default_properties, (copy_ctor_func_t) zval_add_ref,
                   (void *) &tmp, sizeof(zval *));

    return obj;
}

ice_object*
ice_object_get(zval* zv TSRMLS_DC)
{
    if(!zv)
    {
        php_error(E_ERROR, "method %s() must be invoked on an object", get_active_function_name(TSRMLS_C));
        return 0;
    }

    ice_object* obj = (ice_object*)zend_object_store_get_object(zv TSRMLS_CC);
    if(!obj)
    {
        php_error(E_ERROR, "no object found in %s()", get_active_function_name(TSRMLS_C));
        return 0;
    }

    return obj;
}

bool
ice_split_string(const string& str, vector<string>& args)
{
    string delim = " \t\n\r";
    string::size_type beg;
    string::size_type end = 0;
    while(true)
    {
        beg = str.find_first_not_of(delim, end);
        if(beg == string::npos)
        {
            break;
        }

        //
        // Check for quoted argument.
        //
        char ch = str[beg];
        if(ch == '"' || ch == '\'')
        {
            beg++;
            end = str.find(ch, beg);
            if(end == string::npos)
            {
                php_error(E_ERROR, "unterminated quote in `%s'", str.c_str());
                return false;
            }
            args.push_back(str.substr(beg, end - beg));
            end++; // Skip end quote.
        }
        else
        {
            end = str.find_first_of(delim + "'\"", beg);
            if(end == string::npos)
            {
                end = str.length();
            }
            args.push_back(str.substr(beg, end - beg));
        }
    }

    return true;
}

string
ice_lowercase(const string& s)
{
    string result(s);
    transform(result.begin(), result.end(), result.begin(), ::tolower);
    return result;
}
