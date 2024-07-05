# -*- coding: utf-8 -*-
#
# Copyright (c) ZeroC, Inc. All rights reserved.
#

from IcePy import ObjectPrx

#
# Proxy comparison functions.
#

def proxyIdentityEqual(lhs, rhs):
    """Determines whether the identities of two proxies are equal."""
    return proxyIdentityCompare(lhs, rhs) == 0


def proxyIdentityCompare(lhs, rhs):
    """Compares the identities of two proxies."""
    if (lhs and not isinstance(lhs, ObjectPrx)) or (
        rhs and not isinstance(rhs, ObjectPrx)
    ):
        raise ValueError("argument is not a proxy")
    if not lhs and not rhs:
        return 0
    elif not lhs and rhs:
        return -1
    elif lhs and not rhs:
        return 1
    else:
        lid = lhs.ice_getIdentity()
        rid = rhs.ice_getIdentity()
        return (lid > rid) - (lid < rid)


def proxyIdentityAndFacetEqual(lhs, rhs):
    """Determines whether the identities and facets of two
    proxies are equal."""
    return proxyIdentityAndFacetCompare(lhs, rhs) == 0


def proxyIdentityAndFacetCompare(lhs, rhs):
    """Compares the identities and facets of two proxies."""
    if (lhs and not isinstance(lhs, ObjectPrx)) or (
        rhs and not isinstance(rhs, ObjectPrx)
    ):
        raise ValueError("argument is not a proxy")
    if not lhs and not rhs:
        return 0
    elif not lhs and rhs:
        return -1
    elif lhs and not rhs:
        return 1
    elif lhs.ice_getIdentity() != rhs.ice_getIdentity():
        lid = lhs.ice_getIdentity()
        rid = rhs.ice_getIdentity()
        return (lid > rid) - (lid < rid)
    else:
        lf = lhs.ice_getFacet()
        rf = rhs.ice_getFacet()
        return (lf > rf) - (lf < rf)
