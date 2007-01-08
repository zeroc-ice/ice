// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef UTIL_H
#define UTIL_H

#include <string>
#include <sqlca.h>

void handleSqlError(const struct sqlca&, sql_context);

int decodeName(const std::string&);

std::string encodeName(int);

#endif
