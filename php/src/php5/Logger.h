//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

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
