// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_LOGGER_H
#define ICEPHP_LOGGER_H

#include <Config.h>

namespace IcePHP
{

bool loggerInit(void);

bool createLogger(zval*, const Ice::LoggerPtr&);
bool fetchLogger(zval*, Ice::LoggerPtr&);

//
// Class entry.
//
extern zend_class_entry* loggerClassEntry;

} // End of namespace IcePHP

#endif
