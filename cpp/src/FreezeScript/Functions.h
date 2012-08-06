// **********************************************************************
//
// Copyright (c) 2003-2012 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#pragma once

#include <FreezeScript/Data.h>

namespace FreezeScript
{

bool invokeGlobalFunction(const Ice::CommunicatorPtr&, const std::string&, const DataList&, DataPtr&,
                          const DataFactoryPtr&, const ErrorReporterPtr&);

bool invokeMemberFunction(const std::string&, const DataPtr&, const DataList&, DataPtr&, const DataFactoryPtr&,
                          const ErrorReporterPtr&);

}
