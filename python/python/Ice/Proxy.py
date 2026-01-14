# Copyright (c) ZeroC, Inc.

import IcePy

from .ObjectPrx import ObjectPrx

IcePy._t_ObjectPrx = IcePy.defineProxy("::Ice::Object", IcePy.ObjectPrx)

__name__ = "Ice"


#
# Proxy comparison functions.
#


def proxyIdentityEqual(lhs: ObjectPrx | None, rhs: ObjectPrx | None) -> bool:
    """
    Checks if the object identities of two proxies are equal.

    Parameters
    ----------
    lhs : Ice.ObjectPrx | None
        A proxy.
    rhs : Ice.ObjectPrx | None
        A proxy.

    Returns
    -------
    bool
        ``True`` if the identity in ``lhs`` is equal to the identity in ``rhs``,
        ``False`` otherwise.
    """
    return proxyIdentityCompare(lhs, rhs) == 0


def proxyIdentityCompare(lhs: ObjectPrx | None, rhs: ObjectPrx | None) -> int:
    """
    Compares the object identities of two proxies.

    Parameters
    ----------
    lhs : Ice.ObjectPrx | None
        A proxy.
    rhs : Ice.ObjectPrx | None
        A proxy.

    Returns
    -------
    int
        ``-1`` if the identity in ``lhs`` compares less than the identity in ``rhs``;
        ``0`` if the identities compare equal; ``1``, otherwise.
    """
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError("argument is not a proxy")
    if not lhs and not rhs:
        return 0
    elif not lhs and rhs:
        return -1
    elif lhs and not rhs:
        return 1
    else:
        assert lhs is not None and rhs is not None
        lid = lhs.ice_getIdentity()
        rid = rhs.ice_getIdentity()
        return (lid > rid) - (lid < rid)


def proxyIdentityAndFacetEqual(lhs: ObjectPrx | None, rhs: ObjectPrx | None) -> bool:
    """
    Checks if the object identities and facets of two proxies are equal.

    Parameters
    ----------
    lhs : Ice.ObjectPrx | None
        A proxy.
    rhs : Ice.ObjectPrx | None
        A proxy.

    Returns
    -------
    bool
        ``True`` if the identity and facet in ``lhs`` are equal to the identity and facet in ``rhs``,
        ``False`` otherwise.
    """
    return proxyIdentityAndFacetCompare(lhs, rhs) == 0


def proxyIdentityAndFacetCompare(lhs: ObjectPrx | None, rhs: ObjectPrx | None) -> int:
    """
    Compares the object identities and facets of two proxies.

    Parameters
    ----------
    lhs : Ice.ObjectPrx | None
        A proxy.
    rhs : Ice.ObjectPrx | None
        A proxy.

    Returns
    -------
    int
        ``-1`` if the identity and facet in ``lhs`` compare less than the identity and
        facet in ``rhs``; ``0`` if the identities and facets compare equal; ``1``, otherwise.
    """
    if (lhs and not isinstance(lhs, ObjectPrx)) or (rhs and not isinstance(rhs, ObjectPrx)):
        raise ValueError("argument is not a proxy")
    if lhs is None and rhs is None:
        return 0
    elif lhs is None and rhs is not None:
        return -1
    elif lhs is not None and rhs is None:
        return 1
    else:
        assert lhs is not None and rhs is not None
        if lhs.ice_getIdentity() != rhs.ice_getIdentity():
            lid = lhs.ice_getIdentity()
            rid = rhs.ice_getIdentity()
            return (lid > rid) - (lid < rid)
        else:
            lf = lhs.ice_getFacet()
            rf = rhs.ice_getFacet()
            return (lf > rf) - (lf < rf)


__all__ = ["proxyIdentityEqual", "proxyIdentityCompare", "proxyIdentityAndFacetEqual", "proxyIdentityAndFacetCompare"]
