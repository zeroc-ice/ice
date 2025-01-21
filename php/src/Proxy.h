// Copyright (c) ZeroC, Inc.

#ifndef ICEPHP_PROXY_H
#define ICEPHP_PROXY_H

#include "Communicator.h"
#include "Types.h"

namespace IcePHP
{
    bool proxyInit(void);

    bool createProxy(zval*, Ice::ObjectPrx, CommunicatorInfoPtr);
    bool createProxy(zval*, Ice::ObjectPrx, ProxyInfoPtr, CommunicatorInfoPtr);
    bool fetchProxy(zval*, std::optional<Ice::ObjectPrx>&, ProxyInfoPtr&);
    bool fetchProxy(zval*, std::optional<Ice::ObjectPrx>&, ProxyInfoPtr&, CommunicatorInfoPtr&);
    extern zend_class_entry* proxyClassEntry;

} // End of namespace IcePHP

#endif
