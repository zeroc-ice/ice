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

bool createProxy(zval*, const Ice::ObjectPrx&, const CommunicatorInfoPtr&);
bool createProxy(zval*, const Ice::ObjectPrx&, const ProxyInfoPtr&, const CommunicatorInfoPtr&);
bool fetchProxy(zval*, Ice::ObjectPrx&, ProxyInfoPtr&);
bool fetchProxy(zval*, Ice::ObjectPrx&, ProxyInfoPtr&, CommunicatorInfoPtr&);

//
// Class entry.
//
extern zend_class_entry* proxyClassEntry;

} // End of namespace IcePHP

#endif
