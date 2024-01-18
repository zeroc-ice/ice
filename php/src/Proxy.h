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

bool proxyInit(void);

bool createProxy(zval*, Ice::ObjectPrxPtr, CommunicatorInfoPtr);
bool createProxy(zval*, Ice::ObjectPrxPtr, ProxyInfoPtr, CommunicatorInfoPtr);
bool fetchProxy(zval*, Ice::ObjectPrxPtr&, ProxyInfoPtr&);
bool fetchProxy(zval*, Ice::ObjectPrxPtr&, ProxyInfoPtr&, CommunicatorInfoPtr&);

//
// Class entry.
//
extern zend_class_entry* proxyClassEntry;

} // End of namespace IcePHP

#endif
