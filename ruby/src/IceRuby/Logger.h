// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_RUBY_LOGGER_H
#define ICE_RUBY_LOGGER_H

#include <Config.h>
#include <Ice/Logger.h>

namespace IceRuby
{

bool initLogger(VALUE);

//
// Create an object that delegates to a C++ implementation.
//
VALUE createLogger(const Ice::LoggerPtr&);

}

#endif
