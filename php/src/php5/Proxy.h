//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICEPHP_PROXY_H
#define ICEPHP_PROXY_H

#include <Config.h>
#include <Communicator.h>
#include <Types.h>

namespace IcePHP
{

bool proxyInit(TSRMLS_D);

bool createProxy(zval*, const Ice::ObjectPrx&, const CommunicatorInfoPtr& TSRMLS_DC);
bool createProxy(zval*, const Ice::ObjectPrx&, const ProxyInfoPtr&, const CommunicatorInfoPtr& TSRMLS_DC);
bool fetchProxy(zval*, Ice::ObjectPrx&, ProxyInfoPtr& TSRMLS_DC);
bool fetchProxy(zval*, Ice::ObjectPrx&, ProxyInfoPtr&, CommunicatorInfoPtr& TSRMLS_DC);

//
// Class entry.
//
extern zend_class_entry* proxyClassEntry;

} // End of namespace IcePHP

#endif
