// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

bool loggerInit(TSRMLS_D);

bool createLogger(zval*, const Ice::LoggerPtr& TSRMLS_DC);
bool fetchLogger(zval*, Ice::LoggerPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* loggerClassEntry;

} // End of namespace IcePHP

#endif
