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

#ifndef ICE_FACTORYTABLE_H
#define ICE_FACTORYTABLE_H

#include <Ice/FactoryTableDef.h>

namespace Ice
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
