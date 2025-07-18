# Copyright (c) ZeroC, Inc.

# Avoid evaluating annotations at function definition time.
from __future__ import annotations

from collections.abc import Awaitable, Sequence
from typing import TYPE_CHECKING

import IcePy

from .OperationMode import OperationMode
from .StringSeq import _Ice_StringSeq_t

if TYPE_CHECKING:
    from .Current import Current
    from .Identity import Identity


class Object:
    """
    The base class for servants.
    """

    _op_ice_isA = IcePy.Operation(
        "ice_isA",
        "ice_isA",
        OperationMode.Idempotent,
        None,
        (),
        (((), IcePy._t_string, False, 0),),
        (),
        ((), IcePy._t_bool, False, 0),
        (),
    )

    _op_ice_ping = IcePy.Operation(
        "ice_ping",
        "ice_ping",
        OperationMode.Idempotent,
        None,
        (),
        (),
        (),
        None,
        (),
    )

    _op_ice_ids = IcePy.Operation(
        "ice_ids",
        "ice_ids",
        OperationMode.Idempotent,
        None,
        (),
        (),
        (),
        ((), _Ice_StringSeq_t, False, 0),
        (),
    )

    _op_ice_id = IcePy.Operation(
        "ice_id",
        "ice_id",
        OperationMode.Idempotent,
        None,
        (),
        (),
        (),
        ((), IcePy._t_string, False, 0),
        (),
    )

    def ice_isA(self, id: Identity, current: Current) -> bool | Awaitable[bool]:
        """
        Determine whether the target object supports the interface denoted by the given Slice type ID.

        Parameters
        ----------
        id : str
            The Slice type ID.
        current : Ice.Current
            The current context.

        Returns
        -------
        bool
            True if the target object supports the interface, False otherwise.
        """
        ret = self.ice_ids(current)
        if isinstance(ret, Awaitable):
            # wrap and do the check
            async def wrapper():
                ids = await ret
                return id in ids

            return wrapper()
        else:
            return id in ret

    def ice_ping(self, current: Current) -> None | Awaitable[None]:
        """
        A reachability test for the target object.

        Parameters
        ----------
        current : Ice.Current
            The current context.
        """
        pass

    def ice_ids(self, current: Current) -> Sequence[str] | Awaitable[Sequence[str]]:
        """
        Obtain the type IDs corresponding to the Slice interfaces that are supported by the target object.

        Parameters
        ----------
        current : Ice.Current
            The current context.

        Returns
        -------
        Sequence[str]
            A list of type IDs.
        """
        return [Object.ice_staticId()]

    def ice_id(self, current: Current) -> str | Awaitable[str]:
        """
        Obtain the type ID corresponding to the most-derived Slice interface supported by the target object.

        Parameters
        ----------
        current : Ice.Current
            The current context.

        Returns
        -------
        str
            The type ID.
        """
        return Object.ice_staticId()

    @staticmethod
    def ice_staticId() -> str:
        """
        Obtain the type ID of this Slice class or interface.

        Returns
        -------
        str
            The type ID.
        """
        return "::Ice::Object"
