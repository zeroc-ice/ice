// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef FREEZE_CATALOG_DATA_ICE
#define FREEZE_CATALOG_DATA_ICE

[["cpp:header-ext:h"]]

module Freeze
{

/**
 *
 * The catalog keeps information about  Freeze Maps and Freeze
 * evictors in a Berkeley Db environment. It is used by FreezeScript.
 *
 **/

struct CatalogData
{
    bool evictor;
    string key;
    string value;
};

};

#endif
