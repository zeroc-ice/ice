// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef UTIL_H
#define UTIL_H

#include <Ice/Ice.h>
#include <string>
#include <sqlca.h>

void handleSqlError(const sqlca&, sql_context);
void handleNotFound(const Ice::Current&, sql_context);

int decodeName(const std::string&);

std::string encodeName(int);

#endif
