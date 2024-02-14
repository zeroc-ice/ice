//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include "Ice/ProxyFunctions.h"
#include "Reference.h"
#include "LocatorInfo.h"
#include "RouterInfo.h"

using namespace std;
using namespace Ice;
using namespace IceInternal;

ReferencePtr
IceInternal::setLocator(const ObjectPrx& proxy, const std::optional<LocatorPrx>& locator)
{
    const ReferencePtr& currentRef = proxy._getReference();
    ReferencePtr ref = currentRef->changeLocator(locator);
    if (targetEqualTo(ref, currentRef))
    {
        return currentRef;
    }
    else
    {
        return ref;
    }
}

ReferencePtr
IceInternal::setRouter(const ObjectPrx& proxy, const std::optional<RouterPrx>& router)
{
    const ReferencePtr& currentRef = proxy._getReference();
    ReferencePtr ref = currentRef->changeRouter(router);
    if (targetEqualTo(ref, currentRef))
    {
        return currentRef;
    }
    else
    {
        return ref;
    }
}

std::optional<LocatorPrx>
Ice::ice_getLocator(const ObjectPrx& proxy)
{
    LocatorInfoPtr locatorInfo = proxy._getReference()->getLocatorInfo();
    return locatorInfo ? make_optional(locatorInfo->getLocator()) : nullopt;
}

std::optional<RouterPrx>
Ice::ice_getRouter(const ObjectPrx& proxy)
{
    RouterInfoPtr routerInfo = proxy._getReference()->getRouterInfo();
    return routerInfo ? make_optional(routerInfo->getRouter()) : nullopt;
}
