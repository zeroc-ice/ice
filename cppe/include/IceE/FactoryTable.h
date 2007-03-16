// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEE_FACTORYTABLE_H
#define ICEE_FACTORYTABLE_H

#include <IceE/FactoryTableDef.h>

namespace IceInternal
{

class ICE_API FactoryTable
{
public:

    FactoryTable();
    ~FactoryTable();
};

static FactoryTable factoryTableInitializer;	// Dummy variable to force initialization of factoryTable

extern ICE_API FactoryTableDef* factoryTable;

}

#endif
