// **********************************************************************
//
// Copyright (c) 2004
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

#ifndef FREEZE_SCRIPT_FUNCTIONS_H
#define FREEZE_SCRIPT_FUNCTIONS_H

#include <FreezeScript/Data.h>

namespace FreezeScript
{

bool invokeGlobalFunction(const std::string&, const DataList&, DataPtr&, const DataFactoryPtr&,
                          const ErrorReporterPtr&);

bool invokeMemberFunction(const std::string&, const DataPtr&, const DataList&, DataPtr&, const DataFactoryPtr&,
                          const ErrorReporterPtr&);

}

#endif
