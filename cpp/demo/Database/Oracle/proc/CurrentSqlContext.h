// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef CURRENT_SQL_CONTEXT_H
#define CURRENT_SQL_CONTEXT_H

#include <Ice/Ice.h>

//
// Provides a sql_context for each thread created by the communicator(s)
// using the associated thread-notification object
//

//
// Note: sql_context is defined in Pro*C -generated code, so this
// header can only be included in .pc files.
//

class CurrentSqlContext
{
public:
    
    CurrentSqlContext(const std::string&);

    //
    // Default copy ctor and assignment operator ok
    // 

    Ice::ThreadNotificationPtr getHook() const;
    operator sql_context() const;

private:
    const std::string _connectInfo;
    size_t _index;
    Ice::ThreadNotificationPtr _hook;
};

#endif
