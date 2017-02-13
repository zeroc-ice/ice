// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEPHP_PROXY_H
#define ICEPHP_PROXY_H

#include <Config.h>
#include <Communicator.h>
#include <Types.h>

namespace IcePHP
{

bool proxyInit(TSRMLS_D);

bool createProxy(zval*, const Ice::ObjectPrx&, const CommunicatorInfoPtr& TSRMLS_DC);
bool createProxy(zval*, const Ice::ObjectPrx&, const ClassInfoPtr&, const CommunicatorInfoPtr& TSRMLS_DC);
bool fetchProxy(zval*, Ice::ObjectPrx&, ClassInfoPtr& TSRMLS_DC);
bool fetchProxy(zval*, Ice::ObjectPrx&, ClassInfoPtr&, CommunicatorInfoPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* proxyClassEntry;

} // End of namespace IcePHP

#endif
