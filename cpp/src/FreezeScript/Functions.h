// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
