// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_CONNECTION_H
#define ICE_RUBY_CONNECTION_H

#include <Config.h>
#include <Ice/Connection.h>

namespace IceRuby
{

void initConnection(VALUE);

VALUE createConnection(const Ice::ConnectionPtr&);
VALUE createConnectionInfo(const Ice::ConnectionInfoPtr&);

Ice::ConnectionPtr getConnection(VALUE);
bool checkConnection(VALUE);

}

#endif
