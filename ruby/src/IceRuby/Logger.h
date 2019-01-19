//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
